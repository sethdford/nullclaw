# Project Scalpel — Chat Page Architectural Rewrite

**Date:** 2026-03-07
**Status:** Approved
**Scope:** Full decomposition of `chat-view.ts` god object + SOTA feature additions

## Problem

`chat-view.ts` is a 1,179-line god object handling 15+ concerns: message rendering,
gateway/streaming, file upload, search, context menus, keyboard shortcuts, caching,
history, input, scroll, error handling, and more. Critical bugs exist (context menu
never rendered, dead CSS). UX gaps vs Claude.ai/ChatGPT in message actions, session
management, and file handling.

## Target Component Tree

```
sc-chat-view (~200 lines, thin orchestrator)
├── sc-chat-sessions-panel (inline session list, new chat)
├── sc-chat-search (exists)
├── sc-message-list (scroll container, auto-scroll, grouping)
│   ├── sc-message-group (grouped by sender + 2-min window)
│   │   ├── sc-message-stream (exists — content rendering)
│   │   └── sc-message-actions (hover: copy, retry, regenerate, edit)
│   ├── sc-tool-result (exists)
│   └── sc-reasoning-block (exists)
├── sc-composer (input, send, file attach, suggestions)
│   ├── sc-file-preview (attachment thumbnails)
│   └── suggested prompts (empty state)
├── sc-context-menu (exists — wire it)
└── ChatController (reactive controller — gateway, streaming, cache)
```

## New Components

### ChatController (Lit ReactiveController, ~300 lines)

Owns all non-UI logic:

- `items: ChatItem[]` — the unified message/tool/thinking array
- Gateway event handling (chunk, sent, received, thinking)
- Streaming state and timers
- Session cache (sessionStorage) and history loading
- Methods: `send()`, `abort()`, `retry()`, `regenerate()`
- Fully unit-testable without DOM

### sc-composer (~250 lines)

- Auto-resizing textarea (max 5 lines)
- File attachment button (opens file picker)
- Drag-and-drop zone with `sc-file-preview` thumbnails
- Send button (disabled when empty/disconnected/waiting)
- Character count
- Suggested prompt pills (shown in empty state)
- Events: `sc-send`, `sc-abort`, `sc-use-suggestion`

### sc-message-list (~200 lines)

- Scroll container with auto-scroll on new messages
- "New messages" pill when scrolled up
- Message grouping by sender + 2-minute time window
- Renders `sc-message-group` wrappers around consecutive same-sender messages
- Delegates to `sc-message-stream`, `sc-tool-result`, `sc-reasoning-block`

### sc-message-actions (~150 lines)

- Hover bar on each message (appears on mouseenter)
- Actions: Copy (clipboard), Retry (user), Regenerate (assistant), Edit (user)
- Phosphor icons, design token styling
- Events: `sc-copy`, `sc-retry`, `sc-regenerate`, `sc-edit`

### sc-chat-sessions-panel (~250 lines)

- Collapsible panel on left (drawer on mobile)
- Lists recent sessions with titles and timestamps
- "New Chat" button at top
- Rename and delete per session
- Active session highlighted
- Events: `sc-session-select`, `sc-session-new`, `sc-session-delete`

### sc-file-preview (~80 lines)

- Thumbnail grid for attached files
- Image preview for image types
- File icon + name for other types
- Remove button per file
- Events: `sc-file-remove`

## Fixes Included

1. **Context menu**: Wire `<sc-context-menu>` into chat-view render template
2. **Dead CSS**: Remove ~60 lines of unused `.tool-card` styles
3. **Double bubble**: Remove overlapping message styles between chat-view and sc-message-stream
4. **Adaptive code theme**: sc-code-block switches Shiki theme based on prefers-color-scheme

## Phases

### Phase 1: Controller + Bug Fixes

- Extract `ChatController` from chat-view.ts
- Wire context menu into template (fix BROKEN bug)
- Remove dead CSS and double-bubble styles
- Unit tests for ChatController
- chat-view.ts shrinks from 1,179 to ~800 lines

### Phase 2: Component Decomposition

- Extract `sc-composer` (input bar, file upload, suggestions)
- Extract `sc-message-list` (scroll, grouping, rendering)
- Rewrite `chat-view.ts` as thin orchestrator (~200 lines)
- Component tests for sc-composer and sc-message-list
- E2E tests still pass

### Phase 3: SOTA Features

- `sc-message-actions` (hover copy/retry/regenerate/edit)
- `sc-chat-sessions-panel` (inline session list + new chat)
- `sc-file-preview` (attachment thumbnails)
- Message grouping by sender + 2-min time window
- Adaptive code block theme (light/dark)
- Component tests + E2E for new features

## Success Criteria

- chat-view.ts < 250 lines
- All existing E2E tests pass
- Context menu works (right-click copy/retry)
- Per-message hover actions (copy, retry, regenerate)
- Inline session panel with new chat
- 0 dead CSS, 0 double-styling
- All new components have unit tests
