#include "ml_inference.h"

// TFLM headers from Pico-TFLMicro submodule
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Model data generated from Colab (renamed to model_data.cc)
extern unsigned char safety_model_tflite[];
extern unsigned int  safety_model_tflite_len;

// NOTE: If you export scaler mean/std from Colab, paste the real values here.
// For now, these are placeholders (mean = 0, std = 1 => no scaling).
// Replace with your actual numbers later for correct behaviour.
static const float kFeatureMean[4] = {
    0.0f, 0.0f, 0.0f, 0.0f  // LPG, CO, NH3, CO2
};

static const float kFeatureStd[4] = {
    1.0f, 1.0f, 1.0f, 1.0f  // LPG, CO, NH3, CO2
};

namespace {

// Adjust if you get "arena too small" errors at runtime.
constexpr int kTensorArenaSize = 8 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

// Pointers to TFLM objects.
const tflite::Model*          g_model        = nullptr;
tflite::MicroInterpreter*     g_interpreter  = nullptr;
TfLiteTensor*                 g_input_tensor = nullptr;
TfLiteTensor*                 g_output_tensor = nullptr;

}  // namespace

bool ml_inference_init(void) {
    // Map the model from the C array
    g_model = tflite::GetModel(safety_model_tflite);
    if (g_model == nullptr) {
        return false;
    }

    // Schema version check
    if (g_model->version() != TFLITE_SCHEMA_VERSION) {
        return false;
    }

    // Resolver: register only the ops your tiny MLP needs.
    // A dense MLP with ReLU + Softmax typically needs:
    //   - FULLY_CONNECTED
    //   - RELU
    //   - SOFTMAX
    //
    // Increase template parameter if you add more ops.
    static tflite::MicroMutableOpResolver<4> resolver;
    if (resolver.AddFullyConnected() != kTfLiteOk) return false;
    if (resolver.AddRelu()           != kTfLiteOk) return false;
    if (resolver.AddSoftmax()        != kTfLiteOk) return false;
    // Some models insert a RESHAPE op around dense layers. Safe to add:
    resolver.AddReshape();

    // Interpreter (new signature: no ErrorReporter parameter here)
    static tflite::MicroInterpreter static_interpreter(
        g_model,
        resolver,
        tensor_arena,
        kTensorArenaSize
        // MicroResourceVariables* = nullptr
        // MicroProfilerInterface* = nullptr
        // bool use_cur_dylib_error_reporter = false (default)
    );

    g_interpreter = &static_interpreter;

    // Allocate tensors (input / output / intermediates)
    if (g_interpreter->AllocateTensors() != kTfLiteOk) {
        return false;
    }

    g_input_tensor  = g_interpreter->input(0);
    g_output_tensor = g_interpreter->output(0);

    // Basic sanity check: expect 4 inputs, 3 outputs.
    if (g_input_tensor->dims->size != 2 ||
        g_input_tensor->dims->data[1] != 4 ||
        g_input_tensor->type != kTfLiteFloat32) {
        return false;
    }

    if (g_output_tensor->type != kTfLiteFloat32 ||
        g_output_tensor->dims->data[g_output_tensor->dims->size - 1] != 3) {
        return false;
    }

    return true;
}

int ml_inference_run(float lpg, float co, float nh3, float co2) {
    if (!g_interpreter || !g_input_tensor || !g_output_tensor) {
        // Not initialised
        return -1;
    }

    // 1) Standardise using mean/std (same as Colab StandardScaler)
    float raw[4] = { lpg, co, nh3, co2 };

    for (int i = 0; i < 4; ++i) {
        float x = raw[i];
        float mean = kFeatureMean[i];
        float std  = kFeatureStd[i];
        if (std == 0.0f) {
            // Avoid divide-by-zero: treat as no scaling
            g_input_tensor->data.f[i] = x - mean;
        } else {
            g_input_tensor->data.f[i] = (x - mean) / std;
        }
    }

    // 2) Run inference
    if (g_interpreter->Invoke() != kTfLiteOk) {
        return -1;
    }

    // 3) Argmax over 3 outputs (normal, medium, high)
    const float* scores = g_output_tensor->data.f;
    int best_index = 0;
    float best_score = scores[0];

    for (int i = 1; i < 3; ++i) {
        float s = scores[i];
        if (s > best_score) {
            best_score = s;
            best_index = i;
        }
    }

    return best_index;  // 0=normal, 1=medium, 2=high
}
