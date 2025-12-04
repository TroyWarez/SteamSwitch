<div align="center">
  <img src="https://raw.githubusercontent.com/TroyWarez/SteamSwitch/master/SteamSwitchIcon.svg" width="200" height="200"/>
  <h1>Steam Switch</h1>
  <p><b>Seamlessly automate your couch gaming experience on Windows.</b></p>
</div>

---

## ⚠️ Prerequisites
1. **Administrative Privileges:** This project requires Admin privileges to run.
2. **Hardware:** A [libcec](https://github.com/Pulse-Eight/libcec) compatible HDMI CEC USB adapter is required for TV power automation.
3. **Setup:** It is highly recommended to use **Task Scheduler** to run this application automatically from *Program Files* when you log in to Windows.

---

## 🎮 What does it do?
Steam Switch improves the user experience when using a television as your primary display for Steam Big Picture Mode. When Steam exits Big Picture Mode, all changes are automatically reverted to your desktop preferences.

### Key Features
*   **Auto-Hide Cursor:** Automatically hides the mouse cursor to prevent it from obstructing the view.
*   **HDMI CEC Automation:**
    *   Powers on the first HDMI-connected display and sets it as the primary monitor.
    *   Powers off the active HDMI display when exiting Big Picture Mode.
*   **Game Settings Swapper:** Automatically swaps configuration files (graphics/controls) based on your display mode. 
    *   *Example:* Automatically load "Controller + 4K TV" settings when in Big Picture, and "Keyboard + 1440p" settings when at your desk.
*   **Smart Cursor Positioning:** Moves the cursor to the bottom-right corner.
    *   *Why?* This fixes visibility issues in games like **Left 4 Dead 2** where the cursor can break the UI when using a controller, and hides custom game cursors.
*   **Audio Switching:** Automatically toggles the default sound device between your TV and Desktop speakers.

---

## ⚙️ Configuration Guide

### How to set up Game Settings Automation
To enable the automatic swapping of game config files, follow these steps:

1.  **Create Directories:** Create two folders inside `%PROGRAMFILES%\Steam Switch\`:
    *   `\BP\` (For Big Picture/TV settings)
    *   `\DESK\` (For Desktop/Monitor settings)
2.  **Create Game Folders:** Inside both the `BP` and `DESK` folders, create a subfolder with the **exact name** of your target game.
3.  **Define the Path:**
    *   Find the local config path for your game (check [PC Gaming Wiki](https://www.pcgamingwiki.com/)).
    *   Create a file named `path.txt` inside your new game folders.
    *   Paste the path to the game's config folder inside `path.txt` (e.g., `%LocalAppData%\Activision\Call of Duty\players`).
    *   **Important:** Do not use forward slashes (`/`) or leave a trailing backslash at the end of the path.
4.  **Copy Config Files:**
    *   Copy your desired config files into their respective folders (`\BP\GameName\` and `\DESK\GameName\`).
    *   Ensure `path.txt` exists in both.

---

## 🎮 Controller Shortcuts
Use these shortcuts to manage features while in-game or using Steam.

| Button Combination | Hold Time | Action |
| :--- | :--- | :--- |
| **SELECT + D-PAD LEFT** | 5 Seconds | **Disable Cursor Fix:** Use this if the camera spins in circles (e.g., *Portal RTX*). |
| **SELECT + D-PAD UP** | Instant | **Open Keyboard:** Opens the Windows 11 On-Screen Keyboard (Useful for games like *Fallout: New Vegas*). *Note: May not overlay on Exclusive Fullscreen games.* |
| **SELECT + B** | 5 Seconds | **Toggle Audio:** Force switches the default sound device. |

> **Note:** "Select" refers to the **Back** (Xbox 360), **View** (Xbox One/Series), **Share** (PS4), or **Create** (PS5) button.

---

## ✅ Compatibility & Anti-Cheat
Steam Switch has been tested on **Windows 11 (24H2 and later)**. 

It has not triggered issues with standard kernel or non-kernel anti-cheat systems during testing.
