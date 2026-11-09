#pragma once
// Hyprland 0.57 accessors used by gloview.
//
// main targets Hyprland 0.57 ONLY. 0.55.4 and 0.56.x are served by hyprpm.toml's
// commit_pins (the last commits verified on them); this guard fails the build loudly
// instead of producing a .so with an ABI mismatch. 0.57 moved CHLWorkspace to
// workspace/HLWorkspace.hpp (ids became variant<numbered, special>) and CWindow to
// desktop/view/window/Window.hpp.
#if !__has_include(<hyprland/src/workspace/HLWorkspace.hpp>)
#error "gloview main requires Hyprland >= 0.57 headers. Older releases build the hyprpm.toml commit_pins entry."
#endif

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/view/window/Window.hpp>
#include <hyprland/src/desktop/view/LayerSurface.hpp>
#include <hyprland/src/workspace/HLWorkspace.hpp>
#include <hyprland/src/pointer/PointerManager.hpp>
#include <hyprland/src/state/MonitorState.hpp>
#include <hyprland/src/state/MonitorQuery.hpp>
#include <hyprland/src/state/WorkspaceState.hpp>
#include <hyprland/src/state/WorkspaceQuery.hpp>
#include <hyprland/src/desktop/state/WindowState.hpp>
#include <hyprland/src/desktop/state/GlobalWindowController.hpp>
#include <hyprland/src/managers/fullscreen/FullscreenController.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>

#include <cstdint>
#include <optional>

namespace gloview::Compat {

inline const std::vector<PHLMONITOR>& monitors() {
    return State::monitorState()->monitors();
}

inline PHLMONITOR monitorFromCursor() {
    return State::monitorState()->query().vec(g_pInputManager->getMouseCoordsInternal()).run();
}

// 0.57: WorkspaceID is variant<SWorkspaceNumberedID(uint32), SWorkspaceSpecialID>. Named
// NORMAL workspaces carry the SPECIAL-ID variant too, so branch on type()/numberedID(),
// never on a sentinel int. gloview's IDENTITY for a workspace is the object itself
// (PHLWORKSPACE / PHLWORKSPACEREF); the number is only an ORDER key for numbered ones.
inline bool isSpecial(const PHLWORKSPACE& ws) {
    return ws && ws->type() == ::Workspace::eWorkspaceType::SPECIAL;
}

// the workspace's number iff it is a NORMAL numbered workspace (named + special -> nullopt)
inline std::optional<uint32_t> wsNumber(const PHLWORKSPACE& ws) {
    if (!ws || ws->type() != ::Workspace::eWorkspaceType::NORMAL)
        return std::nullopt;
    return ws->numberedID();
}

inline PHLWORKSPACE workspaceByNumber(uint32_t n) {
    if (n == 0)
        return nullptr;
    return State::workspaceState()->query().numbered(::Workspace::SWorkspaceNumberedID{n}).run();
}

// lowest unused workspace number >= from
inline uint32_t lowestFreeNumber(uint32_t from = 1) {
    if (from == 0)
        from = 1;
    while (workspaceByNumber(from))
        ++from;
    return from;
}

// Creates a NUMBERED workspace (nullptr on a duplicate number). 0.57 emits
// workspace.createdLate (Lua `workspace.created`) only from ws->ready(): the caller calls it
// once the workspace is set up, as Hyprland's own actions do. ready() is idempotent.
// Like 0.56's create(), a workspace rule may bind it to a monitor other than `m`.
inline PHLWORKSPACE createNumberedWorkspace(uint32_t n, const PHLMONITOR& m, bool isEmpty) {
    if (n == 0 || !m)
        return nullptr;
    return State::workspaceState()->createNumbered(::Workspace::SWorkspaceNumberedID{n}, m, {}, isEmpty);
}

// weak-ref filtered view over every workspace (iterate + .lock())
inline auto workspaces() {
    return State::workspaceState()->workspaces();
}

inline const std::vector<PHLWINDOW>& windows() {
    return Desktop::windowState()->windows();
}

// the "silent" move (no follow/focus) — gloview handles follow/focus itself
inline void moveWindowToWorkspace(const PHLWINDOW& w, const PHLWORKSPACE& ws) {
    Desktop::globalWindowController()->moveWindowToWorkspace(w, ws);
}

// works for CWindow and CLayerSurface (both derive the geometric-animated mixin)
template <typename T>
inline decltype(auto) realPosition(const SP<T>& v) {
    return v->positionAnimation();
}

template <typename T>
inline decltype(auto) realSize(const SP<T>& v) {
    return v->sizeAnimation();
}

inline bool isInternalFullscreen(const PHLWINDOW& w) { // "fullscreen, not just maximized"
    return Fullscreen::controller()->getFullscreenModes(w).internal == Fullscreen::FSMODE_FULLSCREEN;
}

inline bool isFullscreen(const PHLWINDOW& w) {
    return Fullscreen::controller()->isFullscreen(w);
}

inline auto* pointerMgr() {
    return Pointer::mgr().get();
}

inline PHLANIMVAR<float>& lsAlpha(const PHLLS& ls) {
    return ls->alpha()[Desktop::View::LS_ALPHA_FADE];
}

// CWindow::m_pinned became a bit in the m_state mask (eWindowState) in 0.57.
inline bool isPinned(const PHLWINDOW& w) {
    return w && static_cast<bool>(w->m_state & Desktop::View::WINDOW_STATE_PINNED);
}

} // namespace gloview::Compat
