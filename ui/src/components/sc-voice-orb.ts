import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";
import { icons } from "../icons.js";

export type VoiceOrbState = "idle" | "listening" | "speaking" | "processing" | "unsupported";

@customElement("sc-voice-orb")
export class ScVoiceOrb extends LitElement {
  @property({ type: String }) state: VoiceOrbState = "idle";
  @property({ type: Number }) audioLevel = 0;
  @property({ type: Boolean }) disabled = false;

  static override styles = css`
    :host {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: var(--sc-space-lg) 0 var(--sc-space-md);
      position: relative;
      flex-shrink: 0;
    }

    .mic-orb-glow {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -60%);
      width: 15rem;
      height: 15rem;
      border-radius: 50%;
      background: radial-gradient(circle, var(--sc-accent-subtle) 0%, transparent 70%);
      opacity: 0.4;
      pointer-events: none;
      transition: opacity var(--sc-duration-normal) var(--sc-ease-out);
    }

    .mic-orb-glow.active {
      opacity: 0.8;
      animation: sc-glow-breathe 3s ease-in-out infinite; /* sc-lint-ok: ambient */
    }

    @keyframes sc-glow-breathe {
      0%,
      100% {
        opacity: 0.6;
        transform: translate(-50%, -60%) scale(1);
      }
      50% {
        opacity: 1;
        transform: translate(-50%, -60%) scale(1.1);
      }
    }

    .mic-btn-wrap {
      position: relative;
      z-index: 1;
    }

    .mic-btn {
      width: var(--sc-space-5xl);
      height: var(--sc-space-5xl);
      border-radius: 50%;
      border: 2px solid var(--sc-border);
      background: var(--sc-bg-surface);
      background-image: var(--sc-surface-gradient);
      color: var(--sc-text-muted);
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      position: relative;
      box-shadow: var(--sc-shadow-card);
      transition:
        background var(--sc-duration-fast) var(--sc-ease-out),
        border-color var(--sc-duration-fast) var(--sc-ease-out),
        box-shadow var(--sc-duration-normal) var(--sc-ease-out),
        transform var(--sc-duration-fast) var(--sc-ease-out);
    }

    .mic-btn svg {
      width: 2.5rem;
      height: 2.5rem;
      filter: drop-shadow(0 1px 1px color-mix(in srgb, var(--sc-text) 10%, transparent));
    }

    .mic-btn:hover:not(:disabled) {
      background: var(--sc-bg-elevated);
      border-color: var(--sc-accent);
      color: var(--sc-accent-text, var(--sc-accent));
      box-shadow: var(--sc-shadow-md);
      transform: translateY(-2px);
    }

    .mic-btn:active:not(:disabled) {
      transform: translateY(1px) scaleY(0.97) scaleX(1.01);
    }

    .mic-btn:focus-visible {
      outline: 2px solid var(--sc-accent);
      outline-offset: var(--sc-space-xs);
    }

    .mic-btn:disabled {
      opacity: var(--sc-opacity-disabled, 0.5);
      cursor: not-allowed;
    }

    .mic-btn.active {
      background: var(--sc-accent);
      background-image: var(--sc-button-gradient-primary);
      border-color: var(--sc-accent);
      color: var(--sc-on-accent);
      box-shadow: var(--sc-shadow-glow-accent);
      transform: translateY(-1px);
    }

    .mic-ring {
      position: absolute;
      top: 50%;
      left: 50%;
      width: var(--sc-space-5xl);
      height: var(--sc-space-5xl);
      border-radius: 50%;
      border: 2px solid var(--sc-accent);
      transform: translate(-50%, -50%) scale(1);
      opacity: 0;
      pointer-events: none;
    }

    .mic-ring.active {
      animation: sc-ring-expand 2s ease-out infinite; /* sc-lint-ok: ambient */
    }

    .mic-ring.ring-2.active {
      animation-delay: 0.6s;
    }

    .mic-ring.ring-3.active {
      animation-delay: 1.2s;
    }

    @keyframes sc-ring-expand {
      0% {
        transform: translate(-50%, -50%) scale(1);
        opacity: 0.6;
      }
      100% {
        transform: translate(-50%, -50%) scale(2.2);
        opacity: 0;
      }
    }

    .voice-status {
      margin-top: var(--sc-space-lg);
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      position: relative;
      z-index: 1;
    }

    .voice-status.listening,
    .voice-status.processing {
      color: var(--sc-accent-text, var(--sc-accent));
      font-weight: var(--sc-weight-medium);
    }

    @media (max-width: 30rem) /* --sc-breakpoint-sm */ {
      .mic-btn {
        width: 4.5rem;
        height: 4.5rem;
      }
      .mic-btn svg {
        width: 2rem;
        height: 2rem;
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .mic-btn.active {
        animation: none !important;
      }
      .mic-btn.active {
        box-shadow: var(--sc-shadow-glow-accent);
      }
    }
  `;

  private _handleClick(): void {
    if (this.disabled) return;
    this.dispatchEvent(
      new CustomEvent("sc-voice-mic-toggle", {
        bubbles: true,
        composed: true,
      }),
    );
  }

  private get _statusText(): string {
    switch (this.state) {
      case "listening":
        return "Listening…";
      case "processing":
        return "Processing…";
      case "unsupported":
        return "Speech recognition not supported in this browser";
      default:
        return "Click the microphone to start speaking";
    }
  }

  override render() {
    const isActive = this.state === "listening";
    return html`
      <div class="mic-orb-glow ${isActive ? "active" : ""}"></div>
      <div class="mic-btn-wrap">
        <div class="mic-ring ${isActive ? "active" : ""}"></div>
        <div class="mic-ring ring-2 ${isActive ? "active" : ""}"></div>
        <div class="mic-ring ring-3 ${isActive ? "active" : ""}"></div>
        <button
          class="mic-btn ${isActive ? "active" : ""}"
          ?disabled=${this.disabled}
          @click=${this._handleClick}
          aria-label=${isActive ? "Stop listening" : "Start listening"}
        >
          ${icons.mic}
        </button>
      </div>
      <div class="voice-status ${this.state}" aria-live="polite">${this._statusText}</div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-voice-orb": ScVoiceOrb;
  }
}
