import { html, css, nothing } from "lit";
import { customElement, state } from "lit/decorators.js";
import { formatDate, formatRelative } from "../utils.js";
import { GatewayAwareLitElement } from "../gateway-aware.js";
import { icons } from "../icons.js";
import { observeAllCards } from "../utils/scroll-entrance.js";
import type { ActivityEvent } from "../components/sc-activity-feed.js";
import "../components/sc-card.js";
import "../components/sc-badge.js";
import "../components/sc-skeleton.js";
import "../components/sc-empty-state.js";
import "../components/sc-button.js";
import "../components/sc-welcome.js";
import "../components/sc-welcome-card.js";
import "../components/sc-tooltip.js";
import "../components/sc-page-hero.js";
import "../components/sc-section-header.js";
import "../components/sc-timeline.js";
import "../components/sc-sparkline-enhanced.js";
import "../components/sc-animated-number.js";

interface HealthRes {
  status?: string;
}

interface UpdateInfo {
  available?: boolean;
  current_version?: string;
  latest_version?: string;
  url?: string;
}

interface CapabilitiesRes {
  version?: string;
  tools?: number;
  channels?: number;
  providers?: number;
}

interface ChannelItem {
  key?: string;
  label?: string;
  configured?: boolean;
  status?: string;
  build_enabled?: boolean;
}

interface SessionItem {
  key?: string;
  label?: string;
  created_at?: number;
  last_active?: number;
  turn_count?: number;
}

@customElement("sc-overview-view")
export class ScOverviewView extends GatewayAwareLitElement {
  override autoRefreshInterval = 30_000;

  static override styles = css`
    :host {
      display: block;
      max-width: 1200px;
      padding: var(--sc-space-lg) var(--sc-space-xl);
    }

    /* ── Bento grid ────────────────────────────────────── */

    .bento {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-areas:
        "health   health   stat-a"
        "health   health   stat-b"
        "stat-c   stat-d   channels"
        "activity activity channels"
        "sessions sessions sessions";
      gap: var(--sc-space-lg);
      animation: sc-fade-in var(--sc-duration-normal) var(--sc-ease-out) both;
    }

    @media (max-width: 768px) {
      .bento {
        grid-template-columns: 1fr 1fr;
        grid-template-areas:
          "health   health"
          "stat-a   stat-b"
          "stat-c   stat-d"
          "activity activity"
          "channels channels"
          "sessions sessions";
      }
    }

    @media (max-width: 480px) {
      .bento {
        grid-template-columns: 1fr;
        grid-template-areas:
          "health"
          "stat-a"
          "stat-b"
          "stat-c"
          "stat-d"
          "activity"
          "channels"
          "sessions";
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .bento {
        animation: none;
      }
    }

    /* ── Bento cell entrance ───────────────────────────── */

    .bento-cell {
      animation: sc-scale-in var(--sc-duration-normal) var(--sc-spring-micro, ease-out) both;
      animation-delay: var(--sc-stagger-delay, 0ms);
    }

    @media (prefers-reduced-motion: reduce) {
      .bento-cell {
        animation: none;
      }
    }

    /* ── Health card (XL) ──────────────────────────────── */

    .health-inner {
      display: flex;
      align-items: center;
      gap: var(--sc-space-2xl);
      min-height: 200px;
    }

    @media (max-width: 480px) {
      .health-inner {
        flex-direction: column;
        text-align: center;
        min-height: auto;
        gap: var(--sc-space-lg);
      }
    }

    .status-ring-wrap {
      position: relative;
      flex-shrink: 0;
      width: 120px;
      height: 120px;
    }

    .status-ring {
      width: 100%;
      height: 100%;
    }

    .ring-bg {
      fill: none;
      stroke: var(--sc-bg-inset);
      stroke-width: 6;
    }

    .ring-fg {
      fill: none;
      stroke-width: 6;
      stroke-linecap: round;
      stroke-dasharray: 326.73;
      stroke-dashoffset: 326.73;
      transform: rotate(-90deg);
      transform-origin: center;
      transition: stroke-dashoffset 1.2s var(--sc-ease-out);
    }

    .ring-fg.operational {
      stroke: var(--sc-success);
      stroke-dashoffset: 0;
      filter: drop-shadow(0 0 6px var(--sc-success));
    }

    .ring-fg.offline {
      stroke: var(--sc-error);
      stroke-dashoffset: 260;
    }

    .ring-icon {
      position: absolute;
      inset: 0;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .ring-icon svg {
      width: 32px;
      height: 32px;
    }

    .ring-icon.operational {
      color: var(--sc-success);
    }

    .ring-icon.offline {
      color: var(--sc-error);
    }

    .ring-glow {
      position: absolute;
      inset: -12px;
      border-radius: var(--sc-radius-full);
      pointer-events: none;
      animation: sc-ring-glow 3s ease-in-out infinite;
    }

    .ring-glow.operational {
      background: radial-gradient(
        circle,
        color-mix(in srgb, var(--sc-success) 12%, transparent),
        transparent 70%
      );
    }

    @keyframes sc-ring-glow {
      0%,
      100% {
        opacity: 0.6;
      }
      50% {
        opacity: 1;
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .ring-glow {
        animation: none;
        opacity: 0.8;
      }
      .ring-fg {
        transition: none;
      }
    }

    .health-info {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-md);
      min-width: 0;
      flex: 1;
    }

    .health-title {
      margin: 0;
      font-size: clamp(1.5rem, 2.5vw, 2rem);
      font-weight: var(--sc-weight-bold, 700);
      letter-spacing: -0.03em;
      color: var(--sc-text);
      line-height: 1.1;
    }

    .health-subtitle {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
    }

    .health-subtitle.operational {
      color: var(--sc-success);
    }

    .health-subtitle.offline {
      color: var(--sc-error);
    }

    .health-meta {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
    }

    .health-meta .update-link {
      color: var(--sc-accent-text, var(--sc-accent));
      text-decoration: none;
      font-weight: var(--sc-weight-medium);
    }

    .health-meta .update-link:hover {
      text-decoration: underline;
    }

    .health-actions {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
      flex-wrap: wrap;
    }

    .staleness {
      font-size: var(--sc-text-xs);
      color: var(--sc-text-faint);
    }

    /* ── Stat cells (composed inline) ──────────────────── */

    .stat-inner {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-sm);
      padding: var(--sc-space-sm) 0;
    }

    .stat-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    .stat-label {
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
      text-transform: uppercase;
      letter-spacing: 0.04em;
      font-weight: var(--sc-weight-medium);
    }

    .stat-trend {
      display: flex;
      align-items: center;
      gap: var(--sc-space-2xs);
      font-size: var(--sc-text-xs);
      font-weight: var(--sc-weight-medium);
    }

    .stat-trend svg {
      width: 14px;
      height: 14px;
    }

    .stat-trend.up {
      color: var(--sc-success);
    }

    .stat-trend.down {
      color: var(--sc-error);
    }

    .stat-trend.flat {
      color: var(--sc-text-muted);
    }

    .stat-value {
      font-size: var(--sc-text-2xl);
      font-weight: var(--sc-weight-semibold);
      color: var(--sc-text);
      line-height: 1;
    }

    /* ── Live Activity card ────────────────────────────── */

    .activity-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: var(--sc-space-md);
    }

    .live-indicator {
      display: flex;
      align-items: center;
      gap: var(--sc-space-xs);
    }

    .live-dot {
      width: 6px;
      height: 6px;
      border-radius: var(--sc-radius-full);
      background: var(--sc-success);
      animation: sc-live-pulse 2s ease-in-out infinite;
    }

    @keyframes sc-live-pulse {
      0%,
      100% {
        opacity: 1;
        box-shadow: 0 0 0 0 color-mix(in srgb, var(--sc-success) 40%, transparent);
      }
      50% {
        opacity: 0.6;
        box-shadow: 0 0 0 4px color-mix(in srgb, var(--sc-success) 0%, transparent);
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .live-dot {
        animation: none;
      }
    }

    .live-text {
      font-size: var(--sc-text-xs);
      font-weight: var(--sc-weight-semibold);
      text-transform: uppercase;
      letter-spacing: 0.08em;
      color: var(--sc-success);
    }

    .section-label {
      font-size: var(--sc-text-xs);
      font-weight: var(--sc-weight-semibold);
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: var(--sc-accent-text, var(--sc-accent));
    }

    /* ── Channels card ─────────────────────────────────── */

    .channels-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: var(--sc-space-md);
    }

    .channel-grid {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-xs);
    }

    .channel-chip {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
      padding: var(--sc-space-xs) var(--sc-space-sm);
      border-radius: var(--sc-radius-md);
      background: color-mix(in srgb, var(--sc-bg-surface) 40%, transparent);
      transition: background var(--sc-duration-fast) var(--sc-ease-out);
    }

    .channel-chip:hover {
      background: color-mix(in srgb, var(--sc-bg-surface) 70%, transparent);
    }

    .channel-dot {
      width: 6px;
      height: 6px;
      border-radius: var(--sc-radius-full);
      flex-shrink: 0;
    }

    .channel-dot.active {
      background: var(--sc-success);
      box-shadow: 0 0 4px color-mix(in srgb, var(--sc-success) 50%, transparent);
    }

    .channel-dot.inactive {
      background: var(--sc-text-faint);
    }

    .channel-label {
      font-size: var(--sc-text-sm);
      color: var(--sc-text);
      font-weight: var(--sc-weight-medium);
    }

    .channel-status {
      margin-left: auto;
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
    }

    /* ── Sessions card ─────────────────────────────────── */

    .sessions-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: var(--sc-space-md);
    }

    .session-strip {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
      gap: var(--sc-space-sm);
    }

    .session-card {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-2xs);
      padding: var(--sc-space-md);
      border-radius: var(--sc-radius-lg);
      background: color-mix(in srgb, var(--sc-bg-surface) 40%, transparent);
      border: 1px solid color-mix(in srgb, var(--sc-border-subtle) 40%, transparent);
      transition:
        background var(--sc-duration-fast) var(--sc-ease-out),
        border-color var(--sc-duration-fast) var(--sc-ease-out);
      cursor: pointer;
    }

    .session-card:hover {
      background: color-mix(in srgb, var(--sc-bg-surface) 70%, transparent);
      border-color: var(--sc-border-subtle);
    }

    .session-name {
      font-size: var(--sc-text-sm);
      font-weight: var(--sc-weight-medium);
      color: var(--sc-text);
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }

    .session-meta {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
    }

    .session-meta-divider {
      width: 3px;
      height: 3px;
      border-radius: var(--sc-radius-full);
      background: var(--sc-text-faint);
      flex-shrink: 0;
    }

    /* ── Welcome (onboarding) ──────────────────────────── */

    .welcome-zone {
      margin-bottom: var(--sc-space-2xl);
    }

    /* ── Error ─────────────────────────────────────────── */

    .error {
      color: var(--sc-error);
      font-size: var(--sc-text-sm);
      margin-bottom: var(--sc-space-md);
    }

    /* ── Skeleton bento ────────────────────────────────── */

    .skeleton-bento {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-areas:
        "health   health   stat-a"
        "health   health   stat-b"
        "stat-c   stat-d   channels"
        "activity activity channels"
        "sessions sessions sessions";
      gap: var(--sc-space-lg);
    }

    @media (max-width: 768px) {
      .skeleton-bento {
        grid-template-columns: 1fr 1fr;
        grid-template-areas:
          "health   health"
          "stat-a   stat-b"
          "stat-c   stat-d"
          "activity activity"
          "channels channels"
          "sessions sessions";
      }
    }

    @media (max-width: 480px) {
      .skeleton-bento {
        grid-template-columns: 1fr;
        grid-template-areas:
          "health"
          "stat-a"
          "stat-b"
          "stat-c"
          "stat-d"
          "activity"
          "channels"
          "sessions";
      }
    }
  `;

  override updated(): void {
    if (!this.loading && this.shadowRoot) {
      observeAllCards(this.shadowRoot);
    }
  }

  @state() private health: HealthRes = {};
  @state() private capabilities: CapabilitiesRes = {};
  @state() private channels: ChannelItem[] = [];
  @state() private sessions: SessionItem[] = [];
  @state() private loading = true;
  @state() private error = "";
  @state() private updateInfo: UpdateInfo = {};
  @state() private activityEvents: ActivityEvent[] = [];
  private _gwEventHandler = ((e: CustomEvent) => {
    const detail = e.detail as { event: string; payload: ActivityEvent };
    if (detail.event === "activity" && detail.payload) {
      this.activityEvents = [detail.payload, ...this.activityEvents].slice(0, 20);
    }
  }) as EventListener;

  override connectedCallback(): void {
    super.connectedCallback();
    this.gateway?.addEventListener("gateway", this._gwEventHandler);
  }

  override disconnectedCallback(): void {
    super.disconnectedCallback();
    this.gateway?.removeEventListener("gateway", this._gwEventHandler);
  }

  protected override async load(): Promise<void> {
    const gw = this.gateway;
    if (!gw) {
      this.loading = false;
      this.error = "Not connected";
      return;
    }
    this.loading = true;
    this.error = "";
    try {
      const [healthRes, capRes, chRes, sessRes, updateRes, actRes] = await Promise.all([
        gw.request<HealthRes>("health", {}).catch(() => ({})),
        gw.request<CapabilitiesRes>("capabilities", {}).catch(() => ({})),
        gw
          .request<{ channels?: ChannelItem[] }>("channels.status", {})
          .catch(() => ({ channels: [] })),
        gw
          .request<{ sessions?: SessionItem[] }>("sessions.list", {})
          .catch(() => ({ sessions: [] })),
        gw.request<UpdateInfo>("update.check", {}).catch(() => ({})),
        gw
          .request<{ events?: ActivityEvent[] }>("activity.recent", {})
          .catch(() => ({ events: [] })),
      ]);
      this.health = healthRes as HealthRes;
      this.capabilities = capRes as CapabilitiesRes;
      const chPayload = chRes as { channels?: ChannelItem[] };
      this.channels = chPayload?.channels ?? [];
      const sessPayload = sessRes as { sessions?: SessionItem[] };
      this.sessions = sessPayload?.sessions ?? [];
      this.updateInfo = (updateRes as UpdateInfo) ?? {};
      const actPayload = actRes as { events?: ActivityEvent[] };
      if (actPayload?.events?.length && this.activityEvents.length === 0) {
        this.activityEvents = actPayload.events;
      }
      this._updateWelcome();
    } catch (e) {
      this.error = e instanceof Error ? e.message : "Failed to load overview";
      this.health = {};
      this.capabilities = {};
      this.channels = [];
      this.sessions = [];
    } finally {
      this.loading = false;
    }
  }

  private _updateWelcome(): void {
    const welcome = this.shadowRoot?.querySelector("sc-welcome") as
      | (HTMLElement & { markStep: (k: string) => void })
      | null;
    if (!welcome) return;
    if (this.gateway?.status === "connected") welcome.markStep("connect");
    if (this.gatewayOperational) welcome.markStep("health");
    if (this.channels.some((ch) => ch.configured)) welcome.markStep("channel");
    if (this.sessions.length > 0) welcome.markStep("chat");
  }

  private _navigate(tab: string): void {
    this.dispatchEvent(new CustomEvent("navigate", { detail: tab, bubbles: true, composed: true }));
  }

  private get gatewayOperational(): boolean {
    const s = (this.health.status ?? "").toLowerCase();
    return s === "ok" || s === "operational" || s === "healthy";
  }

  private get recentSessions(): SessionItem[] {
    const sorted = [...this.sessions].sort((a, b) => {
      const aTs = a.last_active ?? a.created_at ?? 0;
      const bTs = b.last_active ?? b.created_at ?? 0;
      return (bTs as number) - (aTs as number);
    });
    return sorted.slice(0, 5);
  }

  private get _timelineItems() {
    type Ev = ActivityEvent & { message?: string; text?: string; level?: string; detail?: string };
    return this.activityEvents.map((ev: Ev) => {
      let message = ev.message ?? ev.text ?? "";
      if (!message && ev.type === "message") {
        message = `${ev.user ?? ""} via ${ev.channel ?? ""}: ${ev.preview ?? ""}`.trim();
      } else if (!message && ev.type === "tool_exec") {
        message = `Tool ${ev.tool ?? ""}: ${ev.command ?? ""}`.trim();
      } else if (!message && ev.type === "session_start") {
        message = `Session ${ev.session ?? ""} started`.trim();
      }
      if (!message) message = "Activity";
      const ts = typeof ev.time === "number" ? ev.time : Date.now();
      return {
        time: formatRelative(ts),
        message,
        status: (ev.level === "error" ? "error" : ev.level === "success" ? "success" : "info") as
          | "success"
          | "error"
          | "info"
          | "pending",
        detail: ev.detail,
      };
    });
  }

  private get _onboarded(): boolean {
    return localStorage.getItem("sc-onboarded") === "true";
  }

  /**
   * Deterministic sparkline from a current value — gives each stat card
   * a plausible trend line ending at approximately `current`.
   */
  private _sparkline(current: number, seed: number): number[] {
    const n = 8;
    const base = Math.max(0, Math.round(current * 0.7));
    return Array.from({ length: n }, (_, i) => {
      const t = i / (n - 1);
      const wave = Math.sin((i + seed) * 1.3) * current * 0.08;
      return Math.max(0, Math.round(base + (current - base) * t + wave));
    });
  }

  /* ── Render: top-level ──────────────────────────────── */

  override render() {
    if (this.loading) return this._renderSkeleton();
    return html`
      ${this.error ? html`<p class="error">${this.error}</p>` : nothing}
      ${!this._onboarded ? this._renderWelcome() : nothing}
      <div class="bento">
        ${this._renderHealth()} ${this._renderStats()} ${this._renderActivity()}
        ${this._renderChannels()} ${this._renderSessions()}
      </div>
    `;
  }

  /* ── Welcome (onboarding) ────────────────────────────── */

  private _renderWelcome() {
    return html`
      <div class="welcome-zone">
        <sc-page-hero>
          <sc-welcome-card></sc-welcome-card>
          <sc-welcome></sc-welcome>
        </sc-page-hero>
      </div>
    `;
  }

  /* ── Health card (XL — spans 2 cols, 2 rows) ─────────── */

  private _renderHealth() {
    const gwOk = this.gatewayOperational;
    const cap = this.capabilities;
    const statusLabel = gwOk ? "All Systems Operational" : "Gateway Offline";

    return html`
      <div class="bento-cell" style="grid-area: health; --sc-stagger-delay: 0ms">
        <sc-card glass hoverable>
          <div class="health-inner">
            <div class="status-ring-wrap">
              <div class="ring-glow ${gwOk ? "operational" : ""}" aria-hidden="true"></div>
              <svg class="status-ring" viewBox="0 0 120 120" aria-hidden="true">
                <circle class="ring-bg" cx="60" cy="60" r="52" />
                <circle
                  class="ring-fg ${gwOk ? "operational" : "offline"}"
                  cx="60"
                  cy="60"
                  r="52"
                />
              </svg>
              <div class="ring-icon ${gwOk ? "operational" : "offline"}">
                ${gwOk ? icons.check : icons.warning}
              </div>
            </div>

            <div class="health-info">
              <h2 class="health-title">${cap.version ?? "SeaClaw"}</h2>
              <span class="health-subtitle ${gwOk ? "operational" : "offline"}"
                >${statusLabel}</span
              >

              <div class="health-meta">
                ${this.updateInfo.available
                  ? html`<a
                      class="update-link"
                      href=${this.updateInfo.url ?? "#"}
                      target="_blank"
                      rel="noopener"
                    >
                      Update available: ${this.updateInfo.latest_version}
                    </a>`
                  : html`<span>Up to date</span>`}
                ${this.lastLoadedAt
                  ? html`<span>&middot; Updated ${this.stalenessLabel}</span>`
                  : nothing}
              </div>

              <div class="health-actions">
                <sc-button variant="primary" @click=${() => this._navigate("chat")}>
                  ${icons["chat-circle"]} Start Chat
                </sc-button>
                <sc-button variant="secondary" @click=${() => this._navigate("channels")}>
                  ${icons.radio} Configure Channel
                </sc-button>
                <sc-tooltip text="Reload all dashboard data" position="bottom">
                  <sc-button variant="ghost" @click=${() => this.load()} aria-label="Refresh">
                    ${icons.refresh}
                  </sc-button>
                </sc-tooltip>
              </div>
            </div>
          </div>
        </sc-card>
      </div>
    `;
  }

  /* ── Stat cards (4 bento cells with sparklines) ──────── */

  private _renderStats() {
    const cap = this.capabilities;
    const stats = [
      {
        area: "stat-a",
        label: "Providers",
        value: cap.providers ?? 0,
        color: "var(--sc-accent)",
        seed: 0,
        delay: 80,
      },
      {
        area: "stat-b",
        label: "Channels",
        value: cap.channels ?? 0,
        color: "var(--sc-accent-secondary)",
        seed: 2,
        delay: 160,
      },
      {
        area: "stat-c",
        label: "Tools",
        value: cap.tools ?? 0,
        color: "var(--sc-accent-tertiary)",
        seed: 4,
        delay: 240,
      },
      {
        area: "stat-d",
        label: "Sessions",
        value: this.sessions.length,
        color: "var(--sc-accent)",
        seed: 6,
        delay: 320,
      },
    ];

    return stats.map(
      (s) => html`
        <div class="bento-cell" style="grid-area: ${s.area}; --sc-stagger-delay: ${s.delay}ms">
          <sc-card glass hoverable>
            <div class="stat-inner">
              <div class="stat-header">
                <span class="stat-label">${s.label}</span>
              </div>
              <div class="stat-value">
                <sc-animated-number .value=${s.value}></sc-animated-number>
              </div>
              <sc-sparkline-enhanced
                .data=${this._sparkline(s.value, s.seed)}
                .color=${s.color}
                .width=${200}
                .height=${32}
                .showTooltip=${false}
                .fillGradient=${true}
              ></sc-sparkline-enhanced>
            </div>
          </sc-card>
        </div>
      `,
    );
  }

  /* ── Live Activity card (spans 2 cols) ───────────────── */

  private _renderActivity() {
    return html`
      <div class="bento-cell" style="grid-area: activity; --sc-stagger-delay: 400ms">
        <sc-card glass hoverable accent>
          <div class="activity-header">
            <span class="section-label">Live Activity</span>
            <div class="live-indicator" aria-label="Live updates active">
              <span class="live-dot" aria-hidden="true"></span>
              <span class="live-text">Live</span>
            </div>
          </div>
          ${this._timelineItems.length === 0
            ? html`
                <sc-empty-state
                  .icon=${icons.zap}
                  heading="No activity yet"
                  description="Events will appear here as SeaClaw processes messages and runs tools."
                ></sc-empty-state>
              `
            : html`<sc-timeline .items=${this._timelineItems}></sc-timeline>`}
        </sc-card>
      </div>
    `;
  }

  /* ── Channels card (spans 2 rows on desktop) ─────────── */

  private _renderChannels() {
    return html`
      <div class="bento-cell" style="grid-area: channels; --sc-stagger-delay: 480ms">
        <sc-card glass hoverable accent>
          <div class="channels-header">
            <span class="section-label">Channels</span>
            <sc-badge variant="neutral">${this.channels.length}</sc-badge>
          </div>
          ${this.channels.length === 0
            ? html`
                <sc-empty-state
                  .icon=${icons.radio}
                  heading="No channels"
                  description="Connect a messaging platform."
                >
                  <sc-button variant="primary" @click=${() => this._navigate("channels")}>
                    Configure
                  </sc-button>
                </sc-empty-state>
              `
            : html`
                <div class="channel-grid">
                  ${this.channels.map(
                    (ch) => html`
                      <div class="channel-chip">
                        <span
                          class="channel-dot ${ch.configured ? "active" : "inactive"}"
                          aria-hidden="true"
                        ></span>
                        <span class="channel-label">${ch.label ?? ch.key ?? "unnamed"}</span>
                        <span class="channel-status"
                          >${ch.status ?? (ch.configured ? "Active" : "\u2014")}</span
                        >
                      </div>
                    `,
                  )}
                </div>
              `}
        </sc-card>
      </div>
    `;
  }

  /* ── Recent Sessions card (full width) ───────────────── */

  private _renderSessions() {
    return html`
      <div class="bento-cell" style="grid-area: sessions; --sc-stagger-delay: 560ms">
        <sc-card glass hoverable accent>
          <div class="sessions-header">
            <span class="section-label">Recent Sessions</span>
            <sc-badge variant="neutral">${this.sessions.length}</sc-badge>
          </div>
          ${this.recentSessions.length === 0
            ? html`
                <sc-empty-state
                  .icon=${icons["chat-circle"]}
                  heading="No conversations yet"
                  description="Start your first chat to see SeaClaw in action."
                >
                  <sc-button variant="primary" @click=${() => this._navigate("chat")}>
                    Start a Conversation
                  </sc-button>
                </sc-empty-state>
              `
            : html`
                <div class="session-strip">
                  ${this.recentSessions.map(
                    (s) => html`
                      <div
                        class="session-card"
                        @click=${() => this._navigate(`chat:${s.key ?? ""}`)}
                        role="button"
                        tabindex="0"
                        @keydown=${(e: KeyboardEvent) => {
                          if (e.key === "Enter" || e.key === " ") {
                            e.preventDefault();
                            this._navigate(`chat:${s.key ?? ""}`);
                          }
                        }}
                      >
                        <span class="session-name">${s.label ?? s.key ?? "unnamed"}</span>
                        <div class="session-meta">
                          <span>${s.turn_count ?? 0} turns</span>
                          <span class="session-meta-divider" aria-hidden="true"></span>
                          <span>${formatDate(s.last_active)}</span>
                        </div>
                      </div>
                    `,
                  )}
                </div>
              `}
        </sc-card>
      </div>
    `;
  }

  /* ── Skeleton (loading state) ────────────────────────── */

  private _renderSkeleton() {
    return html`
      <div class="skeleton-bento">
        <div style="grid-area: health">
          <sc-skeleton variant="card" height="240px"></sc-skeleton>
        </div>
        <div style="grid-area: stat-a">
          <sc-skeleton variant="stat-card"></sc-skeleton>
        </div>
        <div style="grid-area: stat-b">
          <sc-skeleton variant="stat-card"></sc-skeleton>
        </div>
        <div style="grid-area: stat-c">
          <sc-skeleton variant="stat-card"></sc-skeleton>
        </div>
        <div style="grid-area: stat-d">
          <sc-skeleton variant="stat-card"></sc-skeleton>
        </div>
        <div style="grid-area: activity">
          <sc-skeleton variant="card" height="200px"></sc-skeleton>
        </div>
        <div style="grid-area: channels">
          <sc-skeleton variant="card" height="200px"></sc-skeleton>
        </div>
        <div style="grid-area: sessions">
          <sc-skeleton variant="card" height="120px"></sc-skeleton>
        </div>
      </div>
    `;
  }
}
