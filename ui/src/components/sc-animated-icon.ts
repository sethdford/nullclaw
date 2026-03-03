import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";
import { icons } from "../icons.js";

type IconState = "idle" | "loading" | "success" | "error";

@customElement("sc-animated-icon")
export class ScAnimatedIcon extends LitElement {
  @property({ type: String }) icon = "check";
  @property({ type: String }) state: IconState = "idle";
  @property({ type: String }) size = "1.25rem";
  @property({ type: String }) color = "currentColor";

  static override styles = css`
    :host {
      display: inline-flex;
      align-items: center;
      justify-content: center;
    }

    .icon-wrap {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      transition:
        transform var(--sc-duration-normal) var(--sc-spring-out),
        opacity var(--sc-duration-fast) var(--sc-ease-out);
    }
    .icon-wrap svg {
      width: 100%;
      height: 100%;
    }

    .state-loading .icon-wrap {
      animation: sc-icon-spin 800ms linear infinite;
    }

    .state-success .icon-wrap {
      animation: sc-icon-pop var(--sc-duration-normal) var(--sc-spring-bounce);
      color: var(--sc-success);
    }

    .state-error .icon-wrap {
      animation: sc-icon-shake 400ms var(--sc-ease-out);
      color: var(--sc-error);
    }

    @keyframes sc-icon-spin {
      from {
        transform: rotate(0deg);
      }
      to {
        transform: rotate(360deg);
      }
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

    @keyframes sc-icon-shake {
      0%,
      100% {
        transform: translateX(0);
      }
      20% {
        transform: translateX(-3px);
      }
      40% {
        transform: translateX(3px);
      }
      60% {
        transform: translateX(-2px);
      }
      80% {
        transform: translateX(2px);
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .state-loading .icon-wrap,
      .state-success .icon-wrap,
      .state-error .icon-wrap {
        animation: none;
      }
    }
  `;

  private get _icon() {
    if (this.state === "loading") return icons["refresh"] ?? icons["check"];
    if (this.state === "success") return icons["check"];
    if (this.state === "error") return icons["warning"];
    return icons[this.icon] ?? icons["check"];
  }

  override render() {
    return html`
      <div
        class="state-${this.state}"
        style="width: ${this.size}; height: ${this.size}; color: ${this.color}"
      >
        <span class="icon-wrap">${this._icon}</span>
      </div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-animated-icon": ScAnimatedIcon;
  }
}
