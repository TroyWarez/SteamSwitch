<div align="center">
  <img src="https://raw.githubusercontent.com/TroyWarez/SteamSwitch/master/SteamSwitchIcon.svg" width="200" height="200"/>
  <h1>Steam Switch</h1>
  <p><b>Seamlessly automate your couch gaming experience on Windows.</b></p>
</div>

---

## ⚠️ Important Prerequisites
1. **Administrative Privileges:** This project requires Admin privileges to run.
2. **Hardware:** A [libcec](https://github.com/Pulse-Eight/libcec) compatible HDMI CEC USB adapter is required for TV power automation.
3. **Setup:** It is highly recommended to use **Task Scheduler** to run this application automatically from *Program Files* when you log in to Windows.

---

## 🎮 What does it do?
Steam Switch improves the user experience when using a television as your primary display for Steam Big Picture Mode. When Steam exits Big Picture Mode, all changes are automatically reverted.

### Key Features
*   **Auto-Hide Cursor:** Automatically hides the mouse cursor to prevent it from obstructing the view.
*   **HDMI CEC Automation:** Powers on the first HDMI-connected display and sets it as the primary monitor.
*   **Smart Cursor Positioning:** Moves the cursor to the bottom-right corner.
    *   *Why?* This fixes visibility issues in games like **Left 4 Dead 2**, where the cursor can break the UI when using a controller.
*   **Audio Switching:** Automatically toggles the default sound device (configurable via text file).

---

## 🎮 Controller Shortcuts
Use these shortcuts to manage features while in-game or using Steam.

| Button Combination | Hold Time | Action |
| :--- | :--- | :--- |
| **SELECT + D-PAD LEFT** | 5 Seconds | **Disable Cursor Fix:** Use this if the camera spins in circles (e.g., *Portal RTX*). |
| **SELECT + D-PAD UP** | Instant | **Open Keyboard:** Opens the Windows 11 On-Screen Keyboard (Useful for *Fallout: New Vegas* and might not display overtop of games in exclusive fullscreen mode). |
| **SELECT + B** | 5 Seconds | **Toggle Audio:** Switches the default sound device (Might not work correctly when in-game). |

---

## ✅ Compatibility & Anti-Cheat
Steam Switch has been tested on **Windows 11 (24H2/25H2)**. It has not triggered issues with standard kernel or non-kernel anti-cheat systems during testing.

*This project is a work in progress and this readme was made using AI.*
