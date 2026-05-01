# ✨ Emojix

**Emojix** is an ultra-fast, lightweight X11 global keyboard observer and emoji expander written entirely in C. It continuously listens to your keystrokes globally (browser, terminal, chats, text editors) and seamlessly replaces text aliases (e.g., `..smile`) with their corresponding emojis (😄).

![Emojix Logo](logo.svg)

## 🚀 Key Features

*   **⚡ Blazing Fast (Pure C):** Runs efficiently at the native X11 level using the `XRecord` and `Xlib` extensions. Zero latency and practically imperceptible memory/CPU footprint—unlike heavy Electron or Python-based pickers.
*   **🧠 Leventshtein Predictive Autocorrect:** You don't have to spell aliases perfectly! If you make a typo (e.g. `..hrt` instead of `..heart`) and press `Space` or `Enter`, Emojix calculates the closest match using Levenshtein distance, deletes your typo, and types out the corrected emoji instantly (❤️).
*   **🌐 Universal Compatibility:** As a native X11 daemon, it intercepts text universally across *all* applications on your desktop environment.
*   **📚 1,900+ Emojis Included:** Powered by GitHub's official `.gemoji` API database, covering all standard base shortcodes.

## 🛠 Prerequisites

Emojix relies on standard X11 development headers to tap into global keyboard inputs, and uses `xdotool` to simulate the resulting emoji keypresses.

**Arch Linux (or similar distributions):**
```bash
sudo pacman -S base-devel libx11 libxtst xdotool
```

**Debian / Ubuntu:**
```bash
sudo apt install build-essential libx11-dev libxtst-dev xdotool
```

## 🏗 Easiest Installation Workflow

We have streamlined the setup process so you can get Emojix up and running instantly. 

### Step 1: Compile the binary
Navigate to the `emojix` directory and compile the app using `make`. Because we bake all 1,900 emojis natively into the `emojis.h` C-header file beforehand, compilation is instant!

```bash
make
```

### Step 2: Install and Autostart
To make Emojix available from anywhere and ensure it automatically boots up in the background out-of-sight every time you log into your system, simply run:

```bash
make install
```
*Note: This creates a desktop entry in `~/.config/autostart` and moves the binary to your `~/.local/bin/` folder. Root access (`sudo`) is not required!*

### Step 3: Run it right away!
You don't need to use the terminal to start the background daemon! 
Just search for **"Emojix"** in your application launcher (Start Menu) and click the custom SVG Logo 🔥. The background service will spin up silently, and restarting it is as simple as re-clicking the icon.

*(You can also use `pkill emojix` in the terminal if you ever need to stop it manually.)*

## 🎮 How to Use Emojix

Whenever Emojix is running in the background, you can trigger it anywhere you type text!
By default, all emoji hotkeys must start with `..` followed by an alias. The app actively guards your text and **will only expand the emoji when you execute the word by pressing `Spacebar`, `Tab`, or `Enter`!**

**Standard Expansion Examples (Match Exactly):**
*   Type `..smile` + `Space` ➡️ 😄
*   Type `..fire` + `Space` ➡️ 🔥
*   Type `..rocket` + `Space` ➡️ 🚀
*   Type `..pizza` + `Space` ➡️ 🍕

**Fuzzy Autocorrect System:**
If you aren't sure how to spell it, just type what you realistically think it is (starting with `..`) and press the target key (**`Spacebar`**, **`Tab`**, or **`Enter`**). Emojix gracefully deletes your typo and drops in its best prediction!

*   Typing `..smle` + `Space` ➔ 😄
*   Typing `..partyy` + `Space` ➔ 🎉
*   Typing `..hrt` + `Space` ➔ ❤️

## 🧑‍💻 Architecture Notes (Why `generate.py`?)
You'll notice a `generate.py` script in the source code. C is incredibly fast but lacks generic runtime JSON parsing unless bloated third-party libraries are added. To maintain maximum performance, we use `generate.py` to pre-parse huge emoji JSON dictionaries into structured byte arrays within `emojis.h` during development. The user does not need to run this script, it has already done its job providing the C-code.
