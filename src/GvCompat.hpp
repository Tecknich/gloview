#pragma once
// Hyprland 0.55 -> 0.56 API compatibility.
//
// 0.56 moved compositor-owned state into dedicated trackers/controllers:
//   g_pCompositor->m_monitors              -> State::monitorState()->monitors()
//   g_pCompositor->getMonitorFromCursor()  -> State::monitorState()->query().vec(cursor).run()
//   g_pCompositor->getWorkspaceByID(id)    -> State::workspaceState()->query().id(id).run()
//   g_pCompositor->createNewWorkspace(...) -> State::workspaceState()->create(...)
//   g_pCompositor->getWorkspaces()         -> State::workspaceState()->workspaces()
//   g_pCompositor->m_windows               -> Desktop::windowState()->windows()
//   g_pCompositor->moveWindowToWorkspaceSafe -> Desktop::globalWindowController()->moveWindowToWorkspace
//   w->isFullscreen()/isEffectiveInternalFSMode -> Fullscreen::controller()
//   w->m_realPosition / m_realSize (protected) -> positionAnimation()/sizeAnimation()
//   ls->m_alpha                            -> ls->alpha()[Desktop::View::LS_ALPHA_FADE]
//   g_pPointerManager                      -> Pointer::mgr()
// and managers/PointerManager.hpp moved to pointer/PointerManager.hpp.
//
// Detected by header presence (not version macros) so ONE tree builds against both
// the 0.55.4 headers and 0.56+; callers must only use the Compat:: wrappers below.
// The probe is for the OLD path (removed in 0.56): probing a 0.56-only path would
// false-positive whenever newer system headers sit in /usr/include behind a -I'd
// 0.55 prefix (__has_include searches the whole include path, not just the winner).
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/Window.hpp>
#include <hyprland/src/desktop/view/LayerSurface.hpp>

#if !__has_include(<hyprland/src/managers/PointerManager.hpp>)
#define GV_HYPRLAND_056 1
#include <hyprland/src/pointer/PointerManager.hpp>
#include <hyprland/src/state/MonitorState.hpp>
#include <hyprland/src/state/MonitorQuery.hpp>
#include <hyprland/src/state/WorkspaceState.hpp>
#include <hyprland/src/state/WorkspaceQuery.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/state/GlobalWindowController.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#else
#include <hyprland/src/managers/PointerManager.hpp>
#endif

namespace gloview::Compat {

inline const std::vector<PHLMONITOR>& monitors() {
#ifdef GV_HYPRLAND_056
    return State::monitorState()->monitors();
#else
    return g_pCompositor->m_monitors;
#endif
}

inline PHLMONITOR monitorFromCursor() {
#ifdef GV_HYPRLAND_056
    return State::monitorState()->query().vec(g_pInputManager->getMouseCoordsInternal()).run();
#else
    return g_pCompositor->getMonitorFromCursor();
#endif
}

inline PHLWORKSPACE workspaceByID(WORKSPACEID id) {
#ifdef GV_HYPRLAND_056
    return State::workspaceState()->query().id(id).run();
#else
    return g_pCompositor->getWorkspaceByID(id);
#endif
}

inline PHLWORKSPACE createWorkspace(WORKSPACEID id, MONITORID monid, const std::string& name = "", bool isEmpty = true) {
#ifdef GV_HYPRLAND_056
    return State::workspaceState()->create(id, monid, name, isEmpty);
#else
    return g_pCompositor->createNewWorkspace(id, monid, name, isEmpty);
#endif
}

// weak-ref filtered view over every workspace (iterate + .lock(), both versions)
inline auto workspaces() {
#ifdef GV_HYPRLAND_056
    return State::workspaceState()->workspaces();
#else
    return g_pCompositor->getWorkspaces();
#endif
}

inline const std::vector<PHLWINDOW>& windows() {
#ifdef GV_HYPRLAND_056
    return Desktop::windowState()->windows();
#else
    return g_pCompositor->m_windows;
#endif
}

// the "silent" move (no follow/focus) — gloview handles follow/focus itself
inline void moveWindowToWorkspace(const PHLWINDOW& w, const PHLWORKSPACE& ws) {
#ifdef GV_HYPRLAND_056
    Desktop::globalWindowController()->moveWindowToWorkspace(w, ws);
#else
    g_pCompositor->moveWindowToWorkspaceSafe(w, ws);
#endif
}

// works for CWindow and CLayerSurface (both derive the geometric-animated mixin in 0.56)
template <typename T>
inline decltype(auto) realPosition(const SP<T>& v) {
#ifdef GV_HYPRLAND_056
    return v->positionAnimation();
#else
    return (v->m_realPosition);
#endif
}

template <typename T>
inline decltype(auto) realSize(const SP<T>& v) {
#ifdef GV_HYPRLAND_056
    return v->sizeAnimation();
#else
    return (v->m_realSize);
#endif
}

inline bool isInternalFullscreen(const PHLWINDOW& w) { // "fullscreen, not just maximized"
#ifdef GV_HYPRLAND_056
    return Fullscreen::controller()->getFullscreenModes(w).internal == Fullscreen::FSMODE_FULLSCREEN;
#else
    return w->isEffectiveInternalFSMode(FSMODE_FULLSCREEN);
#endif
}

inline bool isFullscreen(const PHLWINDOW& w) {
#ifdef GV_HYPRLAND_056
    return Fullscreen::controller()->isFullscreen(w);
#else
    return w->isFullscreen();
#endif
}

inline auto* pointerMgr() {
#ifdef GV_HYPRLAND_056
    return Pointer::mgr().get();
#else
    return g_pPointerManager.get();
#endif
}

inline PHLANIMVAR<float>& lsAlpha(const PHLLS& ls) {
#ifdef GV_HYPRLAND_056
    return ls->alpha()[Desktop::View::LS_ALPHA_FADE];
#else
    return ls->m_alpha;
#endif
}

} // namespace gloview::Compat
