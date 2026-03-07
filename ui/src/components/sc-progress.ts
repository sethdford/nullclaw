import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";

type ProgressVariant = "default" | "success" | "warning" | "error";
type ProgressSize = "sm" | "md";

const HEIGHT_MAP: Record<ProgressSize, string> = {
  sm: "4px",
  md: "8px",
};

@customElement("sc-progress")
export class ScProgress extends LitElement {
  static override styles = css`
    :host {
      display: block;
      font-family: var(--sc-font);
    }

    .label {
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
      margin-bottom: var(--sc-space-xs);
    }

    .track {
      width: 100%;
      background: var(--sc-bg-elevated);
      border-radius: var(--sc-radius-full);
      overflow: hidden;
    }

    .fill {
      height: 100%;
      border-radius: var(--sc-radius-full);
      transition: width var(--sc-duration-normal) var(--sc-ease-out);
    }

    @media (prefers-reduced-motion: reduce) {
      .fill {
        transition: none;
      }
      .fill.indeterminate {
        animation: none;
        width: 50% !important;
      }
    }

    .fill.variant-default {
      background: var(--sc-accent);
    }

    .fill.variant-success {
      background: var(--sc-success);
    }

    .fill.variant-warning {
      background: var(--sc-warning);
    }

    .fill.variant-error {
      background: var(--sc-error);
    }

    .fill.indeterminate {
      width: 30% !important;
      animation: sc-progress-indeterminate var(--sc-duration-slow) ease-in-out infinite;
    }

    @keyframes sc-progress-indeterminate {
      0% {
        transform: translateX(-100%);
      }
      50% {
        transform: translateX(233%);
      }
      100% {
        transform: translateX(-100%);
      }
    }
  `;

  @property({ type: Number }) value = 0;
  @property({ type: Boolean }) indeterminate = false;
  @property({ type: String }) variant: ProgressVariant = "default";
  @property({ type: String }) size: ProgressSize = "md";
  @property() label = "";

  private get _clampedValue(): number {
    return Math.min(100, Math.max(0, this.value));
  }

  override render() {
    const height = HEIGHT_MAP[this.size];
    const fillWidth = this.indeterminate ? undefined : `${this._clampedValue}%`;

    return html`
      ${this.label
        ? html`
            <div class="label" id="progress-label">
              ${this.label}${!this.indeterminate ? ` ${Math.round(this._clampedValue)}%` : ""}
            </div>
          `
        : null}
      <div
        class="track"
        role="progressbar"
        aria-valuenow=${this.indeterminate ? undefined : this._clampedValue}
        aria-valuemin=${this.indeterminate ? undefined : 0}
        aria-valuemax=${this.indeterminate ? undefined : 100}
        aria-label=${this.label || "Progress"}
        aria-labelledby=${this.label ? "progress-label" : undefined}
        style="height: ${height};"
      >
        <div
          class="fill variant-${this.variant} ${this.indeterminate ? "indeterminate" : ""}"
          style=${fillWidth !== undefined ? `width: ${fillWidth};` : ""}
        ></div>
      </div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-progress": ScProgress;
  }
}
