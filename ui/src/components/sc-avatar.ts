import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";

type AvatarSize = "sm" | "md" | "lg";
type AvatarStatus = "online" | "offline" | "busy" | "away" | "";

const SIZE_MAP: Record<AvatarSize, number> = {
  sm: 28,
  md: 36,
  lg: 48,
};

/** Simple hash of string to number for consistent color derivation */
function hashString(str: string): number {
  let h = 0;
  for (let i = 0; i < str.length; i++) {
    h = (h << 5) - h + str.charCodeAt(i);
    h |= 0;
  }
  return Math.abs(h);
}

/** Generate initials from name: "John Doe" -> "JD", "Alice" -> "A" */
function getInitials(name: string): string {
  const parts = name.trim().split(/\s+/).filter(Boolean);
  if (parts.length === 0) return "?";
  if (parts.length === 1) return parts[0].charAt(0).toUpperCase();
  return (parts[0].charAt(0) + parts[parts.length - 1].charAt(0)).toUpperCase();
}

/** Hue-based colors for initials background (accessible contrast) */
const INITIALS_HUES = [200, 280, 340, 40, 160];

function getInitialsBg(name: string): string {
  const idx = hashString(name) % INITIALS_HUES.length;
  const hue = INITIALS_HUES[idx];
  return `hsl(${hue}, 45%, 35%)`;
}

@customElement("sc-avatar")
export class ScAvatar extends LitElement {
  static override styles = css`
    :host {
      display: inline-flex;
      position: relative;
      font-family: var(--sc-font);
    }

    .avatar {
      border-radius: 50%;
      overflow: hidden;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: var(--sc-weight-semibold);
      color: white;
      flex-shrink: 0;
    }

    .avatar img {
      width: 100%;
      height: 100%;
      object-fit: cover;
    }

    .status-dot {
      position: absolute;
      bottom: 0;
      right: 0;
      border-radius: 50%;
      border: 2px solid var(--sc-bg-overlay);
    }

    .status-dot.online {
      background: var(--sc-success);
    }

    .status-dot.online.pulse {
      animation: sc-avatar-pulse var(--sc-duration-slow) ease-in-out infinite;
    }

    @keyframes sc-avatar-pulse {
      0%,
      100% {
        box-shadow: 0 0 0 0 var(--sc-success);
      }
      50% {
        box-shadow: 0 0 0 4px color-mix(in srgb, var(--sc-success) 30%, transparent);
      }
    }

    @media (prefers-reduced-motion: reduce) {
      .status-dot.online.pulse {
        animation: none;
      }
    }

    .status-dot.offline {
      background: var(--sc-text-muted);
    }

    .status-dot.busy {
      background: var(--sc-error);
    }

    .status-dot.away {
      background: var(--sc-warning);
    }
  `;

  @property() src = "";
  @property() name = "";
  @property({ type: String }) size: AvatarSize = "md";
  @property({ type: String }) status: AvatarStatus = "";

  private _imageError = false;

  override updated(changedProperties: Map<string, unknown>): void {
    if (changedProperties.has("src")) {
      this._imageError = false;
    }
  }

  private get _sizePx(): number {
    return SIZE_MAP[this.size];
  }

  private get _statusDotSize(): number {
    return Math.round(this._sizePx * 0.3);
  }

  private _onImageError = (): void => {
    this._imageError = true;
  };

  override render() {
    const sizePx = this._sizePx;
    const dotSize = this._statusDotSize;
    const showImage = this.src && !this._imageError;
    const initials = getInitials(this.name || "?");
    const initialsBg = getInitialsBg(this.name || "0");

    return html`
      <div
        class="avatar"
        role="img"
        aria-label=${this.name || "Avatar"}
        style="
          width: ${sizePx}px;
          height: ${sizePx}px;
          font-size: ${sizePx * 0.4}px;
          background: ${showImage ? "transparent" : initialsBg};
        "
      >
        ${showImage
          ? html`<img src=${this.src} alt="" @error=${this._onImageError} />`
          : html`<span>${initials}</span>`}
      </div>
      ${this.status
        ? html`
            <div
              class="status-dot ${this.status} ${this.status === "online" ? "pulse" : ""}"
              aria-hidden="true"
              style="
                width: ${dotSize}px;
                height: ${dotSize}px;
              "
            ></div>
          `
        : null}
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-avatar": ScAvatar;
  }
}
