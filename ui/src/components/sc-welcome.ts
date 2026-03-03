import { LitElement, html, css, nothing } from "lit";
import { customElement, state } from "lit/decorators.js";
import { icons } from "../icons.js";
import "./sc-button.js";
import "./sc-card.js";
import "./sc-animated-icon.js";

const ONBOARDED_KEY = "sc-onboarded";

interface OnboardStep {
  key: string;
  icon: string;
  title: string;
  description: string;
  action: string;
  done: boolean;
}

@customElement("sc-welcome")
export class ScWelcome extends LitElement {
  static override styles = css`
    :host {
      display: block;
    }
    .welcome {
      animation: sc-fade-in var(--sc-duration-moderate) var(--sc-ease-out);
    }
    .greeting {
      margin-bottom: var(--sc-space-lg);
    }
    .greeting h2 {
      font-size: var(--sc-text-2xl);
      font-weight: var(--sc-weight-bold);
      letter-spacing: -0.02em;
      color: var(--sc-text);
      margin: 0 0 var(--sc-space-xs);
    }
    .greeting p {
      font-size: var(--sc-text-base);
      color: var(--sc-text-muted);
      margin: 0;
      line-height: var(--sc-leading-relaxed);
    }
    .steps {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(240px, 1fr));
      gap: var(--sc-space-md);
      margin-bottom: var(--sc-space-xl);
    }
    .step-card {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-sm);
      position: relative;
      overflow: hidden;
    }
    .step-icon {
      width: 2rem;
      height: 2rem;
      color: var(--sc-accent);
    }
    .step-icon svg {
      width: 100%;
      height: 100%;
    }
    .step-icon.done {
      color: var(--sc-success);
    }
    .step-title {
      font-size: var(--sc-text-base);
      font-weight: var(--sc-weight-semibold);
      color: var(--sc-text);
    }
    .step-desc {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      line-height: var(--sc-leading-relaxed);
      flex: 1;
    }
    .step-progress {
      position: absolute;
      bottom: 0;
      left: 0;
      right: 0;
      height: 3px;
      border-radius: 0 0 var(--sc-radius-lg) var(--sc-radius-lg);
    }
    .step-progress.done {
      background: linear-gradient(
        90deg,
        var(--sc-success),
        color-mix(in srgb, var(--sc-success) 40%, transparent)
      );
    }
    .step-progress.pending {
      background: color-mix(in srgb, var(--sc-border) 50%, transparent);
    }
    .dismiss {
      text-align: center;
    }
    .dismiss-link {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      background: none;
      border: none;
      cursor: pointer;
      text-decoration: underline;
      text-underline-offset: 2px;
      font-family: var(--sc-font);
      padding: var(--sc-space-sm);
      transition: color var(--sc-duration-fast);
    }
    .dismiss-link:hover {
      color: var(--sc-text);
    }
    .celebration {
      text-align: center;
      padding: var(--sc-space-xl);
    }
    .celebration .check-icon {
      width: 3rem;
      height: 3rem;
      color: var(--sc-success);
      margin: 0 auto var(--sc-space-md);
      animation: sc-icon-pop var(--sc-duration-moderate) var(--sc-spring-bounce);
    }
    .celebration .check-icon svg {
      width: 100%;
      height: 100%;
    }
    @keyframes sc-icon-pop {
      0% {
        transform: scale(0.5);
        opacity: 0;
      }
      50% {
        transform: scale(1.2);
        opacity: 1;
      }
      100% {
        transform: scale(1);
        opacity: 1;
      }
    }
    .celebration h3 {
      font-size: var(--sc-text-lg);
      font-weight: var(--sc-weight-semibold);
      margin: 0 0 var(--sc-space-xs);
    }
    .celebration p {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      margin: 0;
    }
  `;

  @state() private steps: OnboardStep[] = [
    {
      key: "connect",
      icon: "zap",
      title: "Connect Gateway",
      description: "SeaClaw is connecting to your local gateway. This happens automatically.",
      action: "overview",
      done: false,
    },
    {
      key: "health",
      icon: "shield",
      title: "Verify Health",
      description: "Check that all subsystems are operational and responding.",
      action: "overview",
      done: false,
    },
    {
      key: "channel",
      icon: "radio",
      title: "Configure a Channel",
      description: "Set up Telegram, Discord, Slack, or any messaging channel.",
      action: "channels",
      done: false,
    },
    {
      key: "chat",
      icon: "message-square",
      title: "Start a Conversation",
      description: "Send your first message and see SeaClaw in action.",
      action: "chat",
      done: false,
    },
  ];
  @state() private dismissed = false;
  @state() private allDone = false;

  get visible(): boolean {
    if (this.dismissed) return false;
    return localStorage.getItem(ONBOARDED_KEY) !== "true";
  }

  markStep(key: string): void {
    this.steps = this.steps.map((s) => (s.key === key ? { ...s, done: true } : s));
    if (this.steps.every((s) => s.done)) {
      this.allDone = true;
      setTimeout(() => {
        localStorage.setItem(ONBOARDED_KEY, "true");
      }, 3000);
    }
  }

  private _dismiss(): void {
    this.dismissed = true;
    localStorage.setItem(ONBOARDED_KEY, "true");
  }

  private _navigate(tab: string): void {
    this.dispatchEvent(new CustomEvent("navigate", { detail: tab, bubbles: true, composed: true }));
  }

  override render() {
    if (!this.visible) return nothing;

    if (this.allDone) {
      return html`
        <sc-card elevated class="celebration">
          <div class="check-icon">${icons.check}</div>
          <h3>You're all set!</h3>
          <p>SeaClaw is configured and ready. Explore the dashboard to manage your AI assistant.</p>
        </sc-card>
      `;
    }

    const done = this.steps.filter((s) => s.done).length;
    const total = this.steps.length;

    return html`
      <div class="welcome">
        <div class="greeting">
          <h2>Welcome to SeaClaw</h2>
          <p>Let's get you set up. ${done}/${total} steps complete.</p>
        </div>
        <div class="steps sc-stagger">
          ${this.steps.map(
            (step) => html`
              <sc-card hoverable class="step-card" @click=${() => this._navigate(step.action)}>
                <div class="step-icon ${step.done ? "done" : ""}">
                  ${step.done ? icons.check : icons[step.icon]}
                </div>
                <div class="step-title">${step.title}</div>
                <div class="step-desc">${step.description}</div>
                ${!step.done
                  ? html`<sc-button variant="secondary" size="sm">Go</sc-button>`
                  : nothing}
                <div class="step-progress ${step.done ? "done" : "pending"}"></div>
              </sc-card>
            `,
          )}
        </div>
        <div class="dismiss">
          <button class="dismiss-link" @click=${this._dismiss}>
            Skip setup — I know what I'm doing
          </button>
        </div>
      </div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-welcome": ScWelcome;
  }
}
