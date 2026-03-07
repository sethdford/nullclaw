import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";
import { formatDate } from "../utils.js";
import { icons } from "../icons.js";
import "../components/sc-card.js";
import "../components/sc-empty-state.js";
import "../components/sc-button.js";

export interface SessionItem {
  key?: string;
  label?: string;
  created_at?: number;
  last_active?: number;
  turn_count?: number;
}

@customElement("sc-sessions-table")
export class ScSessionsTable extends LitElement {
  @property({ type: Array }) sessions: SessionItem[] = [];
  @property({ type: String }) emptyHeading = "No conversations yet";
  @property({ type: String }) emptyDescription = "Start your first chat to see SeaClaw in action.";
  @property({ type: String }) emptyActionLabel = "Start a Conversation";
  @property({ type: String }) emptyActionTarget = "chat";

  static override styles = css`
    :host {
      display: block;
    }

    .sessions-table {
      width: 100%;
      border-collapse: collapse;
    }

    .sessions-table th,
    .sessions-table td {
      padding: var(--sc-space-sm) var(--sc-space-md);
      text-align: left;
      border-bottom: 1px solid var(--sc-border);
      font-size: var(--sc-text-sm);
    }

    .sessions-table th {
      font-size: var(--sc-text-xs);
      font-weight: var(--sc-weight-medium);
      color: var(--sc-text-muted);
    }

    .sessions-table tr:last-child td {
      border-bottom: none;
    }

    .session-row {
      cursor: pointer;
      transition: background-color var(--sc-duration-fast) var(--sc-ease-out);
    }

    .session-row:hover {
      background-color: var(--sc-bg-elevated);
    }

    .session-row:focus-visible {
      outline: 2px solid var(--sc-accent);
      outline-offset: -2px;
    }

    @media (prefers-reduced-motion: reduce) {
      .session-row {
        transition: none;
      }
    }
  `;

  private _onSessionClick(session: SessionItem): void {
    const key = session.key ?? "";
    this.dispatchEvent(
      new CustomEvent("session-select", {
        detail: { key },
        bubbles: true,
        composed: true,
      }),
    );
  }

  override render() {
    if (this.sessions.length === 0) {
      return html`
        <sc-empty-state
          .icon=${icons["chat-circle"]}
          heading=${this.emptyHeading}
          description=${this.emptyDescription}
        >
          <sc-button
            variant="primary"
            @click=${() =>
              this.dispatchEvent(
                new CustomEvent("navigate", {
                  detail: this.emptyActionTarget,
                  bubbles: true,
                  composed: true,
                }),
              )}
          >
            ${this.emptyActionLabel}
          </sc-button>
        </sc-empty-state>
      `;
    }

    return html`
      <table class="sessions-table">
        <thead>
          <tr>
            <th>Session</th>
            <th>Turns</th>
            <th>Last active</th>
          </tr>
        </thead>
        <tbody>
          ${this.sessions.map(
            (s) => html`
              <tr
                class="session-row"
                role="button"
                tabindex="0"
                aria-label=${`Open session ${s.label ?? s.key ?? "unnamed"}`}
                @click=${() => this._onSessionClick(s)}
                @keydown=${(e: KeyboardEvent) => {
                  if (e.key === "Enter" || e.key === " ") {
                    e.preventDefault();
                    this._onSessionClick(s);
                  }
                }}
              >
                <td>${s.label ?? s.key ?? "unnamed"}</td>
                <td>${s.turn_count ?? 0}</td>
                <td>${formatDate(s.last_active)}</td>
              </tr>
            `,
          )}
        </tbody>
      </table>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-sessions-table": ScSessionsTable;
  }
}
