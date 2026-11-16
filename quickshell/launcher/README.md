# Unified Launcher (Quickshell)

MVP scaffold for the Arch Linux + Hyprland unified launcher PRD.

## Structure
- `shell.qml` - launcher window, input, unified grouped results, keyboard handling
- `ui/ResultSection.qml` - grouped result section UI
- `lib/SearchOrchestrator.qml` - debounced async provider orchestration
- `lib/Actions.qml` - execute/copy/open actions
- `providers/search_apps.py` - .desktop application search
- `providers/search_files.py` - filename/path search in configured roots
- `providers/calc.py` - safe math expression evaluation
- `launch.sh` - toggle/start launcher

## Run
```bash
~/.config/quickshell/launcher/launch.sh
# or
qs -c ~/.config/quickshell/launcher
```

## Notes
- Requires Quickshell (`qs`) with QML/Wayland support, `python3`, `jq`/coreutils not required for providers, and `xdg-open`/`wl-copy` or `xclip` for actions.
- File search roots currently: `~/Documents`, `~/Downloads`, `~/Desktop`, `~/Projects`, `~/Pictures` (edit `providers/search_files.py`).
- This is an MVP scaffold; refine QML API usage to match your installed Quickshell version if needed.
