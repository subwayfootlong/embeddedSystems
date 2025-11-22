
---

# Installing SD Card Support (FatFs SPI Driver)

This project uses the **no-OS-FatFS-SD-SPI-RPi-Pico** library to provide SD card access over SPI.
Follow these steps to install it at the correct folder level.

---

## 1. Clone the SD card library

From your **project root directory**, run:

```sh
git clone https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico.git
```

Your folder structure should now look like this:

```
/project-root
    /Pico3
    /no-OS-FatFS-SD-SPI-RPi-Pico
```

---

## 2. No submodule setup required

This library does **not** use git submodules.
You can use it immediately after cloning.

---

## 3. Ensure your Pico project includes and links it

Your Pico project's CMakeLists.txt should contain:

```
add_subdirectory(../no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI FatFs_SPI)
target_link_libraries(pico3 FatFs_SPI)
```

---

