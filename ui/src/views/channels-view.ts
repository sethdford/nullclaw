import { html, css, nothing } from "lit";
import { customElement, state } from "lit/decorators.js";
import { GatewayAwareLitElement } from "../gateway-aware.js";
import { icons } from "../icons.js";
import "../components/sc-card.js";
import "../components/sc-skeleton.js";
import "../components/sc-empty-state.js";
import "../components/sc-animated-icon.js";
import "../components/sc-badge.js";

interface ChannelStatus {
  key?: string;
  label?: string;
  name?: string;
  status?: string;
  healthy?: boolean;
  configured?: boolean;
  build_enabled?: boolean;
  error?: string;
}

@customElement("sc-channels-view")
export class ScChannelsView extends GatewayAwareLitElement {
  static override styles = css`
    :host {
      display: block;
      max-width: 1200px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
      gap: var(--sc-space-xl);
    }
    .grid-full {
      grid-column: 1 / -1;
    }
    .card-header {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
      margin-bottom: var(--sc-space-sm);
    }
    .card-name {
      font-weight: var(--sc-weight-semibold);
      font-size: var(--sc-text-lg);
      color: var(--sc-text);
      flex: 1;
    }
    .status-indicator {
      display: flex;
      align-items: center;
      gap: var(--sc-space-2xs);
      padding: var(--sc-space-2xs) var(--sc-space-sm);
      border-radius: var(--sc-radius-full);
      font-size: var(--sc-text-xs);
      font-weight: var(--sc-weight-medium);
    }
    .status-indicator.healthy {
      background: color-mix(in srgb, var(--sc-success) 12%, transparent);
      color: var(--sc-success);
    }
    .status-indicator.error {
      background: color-mix(in srgb, var(--sc-error) 12%, transparent);
      color: var(--sc-error);
    }
    .status-indicator.unconfigured {
      background: color-mix(in srgb, var(--sc-text-muted) 12%, transparent);
      color: var(--sc-text-muted);
    }
    .status-indicator .dot {
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background: currentColor;
    }
    .status-indicator.healthy .dot {
      box-shadow: 0 0 4px currentColor;
    }
    .card-info {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      margin-top: var(--sc-space-xs);
    }
    .card-info .error-msg {
      color: var(--sc-error);
    }
    .card-accent {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 3px;
      border-radius: var(--sc-radius-lg) var(--sc-radius-lg) 0 0;
    }
    .card-accent.healthy {
      background: linear-gradient(
        90deg,
        var(--sc-success),
        color-mix(in srgb, var(--sc-success) 40%, transparent)
      );
    }
    .card-accent.error {
      background: linear-gradient(
        90deg,
        var(--sc-error),
        color-mix(in srgb, var(--sc-error) 40%, transparent)
      );
    }
    @media (max-width: 768px) {
      .grid {
        grid-template-columns: 1fr 1fr;
      }
    }
    @media (max-width: 480px) {
      .grid {
        grid-template-columns: 1fr;
      }
    }
  `;

  @state() private channels: ChannelStatus[] = [];
  @state() private loading = true;
  @state() private error = "";

  protected override async load(): Promise<void> {
    await this.loadChannels();
  }

  private async loadChannels(): Promise<void> {
    if (!this.gateway) {
      this.loading = false;
      return;
    }
    this.loading = true;
    try {
      const payload = await this.gateway.request<{
        channels?: ChannelStatus[];
      }>("channels.status", {});
      this.channels = payload?.channels ?? [];
    } catch (e) {
      this.channels = [];
      this.error = e instanceof Error ? e.message : "Failed to load channels";
    } finally {
      this.loading = false;
    }
  }

  private dotClass(ch: ChannelStatus): string {
    if (ch.healthy === true || ch.configured === true) return "healthy";
    if (ch.build_enabled === false) return "unconfigured";
    if (ch.configured === false || ch.status === "unconfigured") return "unconfigured";
    return "error";
  }

  override render() {
    if (this.loading) {
      return html`
        <div class="grid sc-stagger">
          <sc-skeleton variant="channel-card"></sc-skeleton>
          <sc-skeleton variant="channel-card"></sc-skeleton>
          <sc-skeleton variant="channel-card"></sc-skeleton>
          <sc-skeleton variant="channel-card"></sc-skeleton>
          <sc-skeleton variant="channel-card"></sc-skeleton>
          <sc-skeleton variant="channel-card"></sc-skeleton>
        </div>
      `;
    }

    return html`
      ${this.error
        ? html`<sc-empty-state
            .icon=${icons.warning}
            heading="Error"
            description=${this.error}
          ></sc-empty-state>`
        : nothing}
      <div class="grid sc-stagger">
        ${this.channels.length === 0
          ? html`
              <div class="grid-full">
                <sc-empty-state
                  .icon=${icons.radio}
                  heading="No channels configured"
                  description="Configure messaging channels to receive and send messages."
                ></sc-empty-state>
              </div>
            `
          : this.channels.map(
              (ch) => html`
                <sc-card hoverable style="position: relative; overflow: hidden;">
                  ${this.dotClass(ch) !== "unconfigured"
                    ? html`<div class="card-accent ${this.dotClass(ch)}" aria-hidden="true"></div>`
                    : nothing}
                  <div class="card-header">
                    <span class="card-name">${ch.label || ch.key || ch.name || "unnamed"}</span>
                    <span class="status-indicator ${this.dotClass(ch)}">
                      <span class="dot" aria-hidden="true"></span>
                      ${this.dotClass(ch) === "healthy"
                        ? "Active"
                        : this.dotClass(ch) === "error"
                          ? "Error"
                          : "Inactive"}
                    </span>
                  </div>
                  <div class="card-info">
                    ${ch.error
                      ? html`<span class="error-msg">${ch.error}</span>`
                      : html`${ch.status ?? "Not configured"}`}
                  </div>
                </sc-card>
              `,
            )}
      </div>
    `;
  }
}
