import { LitElement, html, css } from "lit";
import { customElement, property, state } from "lit/decorators.js";
import { unsafeHTML } from "lit/directives/unsafe-html.js";
import DOMPurify from "dompurify";
import "./sc-toast.js";
import { ScToast } from "./sc-toast.js";

const SHIKI_LANGS = new Set([
  "javascript",
  "typescript",
  "python",
  "bash",
  "json",
  "html",
  "css",
  "c",
  "rust",
  "go",
  "sql",
  "yaml",
  "markdown",
  "shell",
  "jsx",
  "tsx",
  "java",
  "ruby",
  "php",
  "swift",
  "kotlin",
  "zig",
]);

@customElement("sc-code-block")
export class ScCodeBlock extends LitElement {
  @property({ type: String }) code = "";
  @property({ type: String }) language = "";
  @property({ type: Object }) onCopy?: (code: string) => void;

  @state() private _highlighted = "";
  @state() private _copied = false;
  @state() private _shikiReady = false;
  @state() private _darkScheme = true;
  private _copyTimeout = 0;
  private _mediaQuery: MediaQueryList | null = null;
  private _mediaHandler: (() => void) | null = null;
  private _themeObserver: MutationObserver | null = null;

  static override styles = css`
    :host {
      display: block;
      position: relative;
      font-family: var(--sc-font-mono);
      font-size: var(--sc-text-sm);
      background: var(--sc-bg-inset);
      border-radius: var(--sc-radius-md);
      overflow: hidden;
      margin: var(--sc-space-sm) 0;
    }

    .header {
      display: flex;
      align-items: center;
      padding: var(--sc-space-xs) var(--sc-space-sm);
      background: color-mix(in srgb, var(--sc-border) 20%, var(--sc-bg-inset));
      border-bottom: 1px solid var(--sc-border-subtle);
      font-family: var(--sc-font);
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
    }

    .lang-label {
      text-transform: lowercase;
    }

    .copy-btn {
      position: absolute;
      top: var(--sc-space-sm);
      right: var(--sc-space-sm);
      display: flex;
      align-items: center;
      gap: var(--sc-space-2xs);
      padding: var(--sc-space-2xs) var(--sc-space-sm);
      font-family: var(--sc-font);
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
      background: color-mix(in srgb, var(--sc-bg-surface) 65%, transparent);
      backdrop-filter: blur(var(--sc-glass-subtle-blur, 12px));
      -webkit-backdrop-filter: blur(var(--sc-glass-subtle-blur, 12px));
      border: 1px solid color-mix(in srgb, white 8%, transparent);
      border-radius: var(--sc-radius-full);
      cursor: pointer;
      opacity: 0;
      transition:
        opacity var(--sc-duration-fast) var(--sc-ease-out),
        color var(--sc-duration-fast) var(--sc-ease-out),
        border-color var(--sc-duration-fast) var(--sc-ease-out),
        background var(--sc-duration-fast) var(--sc-ease-out);
    }

    :host:hover .copy-btn,
    .copy-btn:focus,
    .copy-btn.copied {
      opacity: 1;
    }

    .copy-btn:hover {
      color: var(--sc-text);
    }

    .copy-btn:focus-visible {
      outline: 2px solid var(--sc-accent);
      outline-offset: 2px;
    }

    .copy-btn.copied {
      color: var(--sc-success);
      border-color: var(--sc-success);
    }

    .content {
      padding: var(--sc-space-md);
      overflow-x: auto;
    }

    .content pre {
      margin: 0;
      white-space: pre;
    }

    .content :global(code) {
      font-family: var(--sc-font-mono);
      font-size: inherit;
    }

    @media (prefers-reduced-transparency: reduce) {
      .copy-btn {
        backdrop-filter: none;
        -webkit-backdrop-filter: none;
        background: var(--sc-bg-elevated);
      }
    }
    @media (prefers-reduced-motion: reduce) {
      .copy-btn {
        transition: none;
      }
    }
  `;

  override connectedCallback(): void {
    super.connectedCallback();
    this._darkScheme = this._resolveScheme();
    this._highlight();
    this._mediaQuery = window.matchMedia("(prefers-color-scheme: dark)");
    this._mediaHandler = () => {
      this._darkScheme = this._resolveScheme();
      this._highlight();
    };
    this._mediaQuery?.addEventListener("change", this._mediaHandler);
    this._themeObserver = new MutationObserver(() => {
      this._darkScheme = this._resolveScheme();
      this._highlight();
    });
    this._themeObserver.observe(document.documentElement, {
      attributes: true,
      attributeFilter: ["data-theme"],
    });
  }

  override disconnectedCallback(): void {
    clearTimeout(this._copyTimeout);
    if (this._mediaQuery && this._mediaHandler) {
      this._mediaQuery.removeEventListener("change", this._mediaHandler);
    }
    this._themeObserver?.disconnect();
    this._mediaQuery = null;
    this._mediaHandler = null;
    this._themeObserver = null;
    super.disconnectedCallback();
  }

  private _resolveScheme(): boolean {
    const explicit = document.documentElement.dataset.theme;
    if (explicit === "dark") return true;
    if (explicit === "light") return false;
    return window.matchMedia("(prefers-color-scheme: dark)").matches;
  }

  override updated(changed: Map<string, unknown>): void {
    if (changed.has("code") || changed.has("language")) {
      this._highlight();
    }
  }

  private async _highlight(): Promise<void> {
    const lang = this.language.toLowerCase().trim();
    const supported = lang && SHIKI_LANGS.has(lang);
    if (!supported) {
      this._highlighted = "";
      this._shikiReady = true;
      return;
    }
    try {
      const { codeToHtml } = await import("shiki");
      const theme = this._darkScheme ? "github-dark-default" : "github-light-default";
      const html = await codeToHtml(this.code, {
        lang,
        theme,
      });
      this._highlighted = html;
    } catch {
      this._highlighted = "";
    }
    this._shikiReady = true;
  }

  private async _onCopy(): Promise<void> {
    if (this.onCopy) {
      this.onCopy(this.code);
      this._copied = true;
      this._copyTimeout = window.setTimeout(() => {
        this._copied = false;
        this.requestUpdate();
      }, 2000);
      return;
    }
    try {
      await navigator.clipboard.writeText(this.code);
      ScToast.show({ message: "Copied to clipboard", variant: "success", duration: 2000 });
      this._copied = true;
      this._copyTimeout = window.setTimeout(() => {
        this._copied = false;
        this.requestUpdate();
      }, 2000);
    } catch {
      ScToast.show({ message: "Failed to copy", variant: "error" });
    }
  }

  override render() {
    const langLabel = this.language ? this.language.toLowerCase() : "plain";
    const showHighlighted = this._shikiReady && this._highlighted;
    return html`
      <div
        class="code-block"
        role="region"
        aria-label=${`Code block${this.language ? ` (${langLabel})` : ""}`}
      >
        <div class="header">
          <span class="lang-label">${langLabel}</span>
        </div>
        <button
          type="button"
          class="copy-btn ${this._copied ? "copied" : ""}"
          aria-label=${this._copied ? "Copied" : "Copy code"}
          @click=${this._onCopy}
        >
          ${this._copied ? "Copied!" : "Copy"}
        </button>
        <div class="content">
          ${showHighlighted
            ? html`${unsafeHTML(DOMPurify.sanitize(this._highlighted))}`
            : html`<pre><code>${this.code}</code></pre>`}
        </div>
      </div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-code-block": ScCodeBlock;
  }
}
