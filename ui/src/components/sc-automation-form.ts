import { LitElement, html, css, nothing } from "lit";
import { customElement, property, state } from "lit/decorators.js";
import "./sc-input.js";
import "./sc-textarea.js";
import "./sc-button.js";
import "./sc-form-group.js";
import "./sc-schedule-builder.js";

export interface AutomationTemplate {
  name: string;
  description: string;
  type: "agent" | "shell";
  prompt?: string;
  command?: string;
  expression: string;
  channel?: string;
  icon?: string;
}

export interface CronJob {
  id: number;
  name: string;
  expression: string;
  command: string;
  type: "agent" | "shell";
  channel?: string;
  enabled: boolean;
  paused: boolean;
  one_shot?: boolean;
  next_run?: number;
  last_run?: number;
  last_status?: string;
  created_at?: number;
}

export interface ChannelStatus {
  name: string;
  key: string;
  configured: boolean;
}

export interface AutomationFormData {
  type: "agent" | "shell";
  name: string;
  expression: string;
  one_shot: boolean;
  prompt?: string;
  command?: string;
  channel?: string;
}

export const TEMPLATES: AutomationTemplate[] = [
  {
    name: "Daily Digest",
    description: "Summarize yesterday's messages across all channels",
    type: "agent",
    prompt:
      "Summarize all messages I received yesterday across all channels. Highlight anything urgent or requiring my attention.",
    expression: "0 8 * * *",
    icon: "summary",
  },
  {
    name: "Weekly Report",
    description: "Generate a weekly activity summary every Friday",
    type: "agent",
    prompt:
      "Generate a comprehensive weekly report summarizing: conversations handled, tools used, tasks completed, and any recurring topics or issues from this week.",
    expression: "0 17 * * 5",
    icon: "report",
  },
  {
    name: "Email Monitor",
    description: "Check for urgent emails every hour during business hours",
    type: "agent",
    prompt:
      "Check my email for any urgent or time-sensitive messages. If you find any, summarize them and alert me.",
    expression: "0 9-17 * * 1-5",
    channel: "gmail",
    icon: "email",
  },
  {
    name: "Health Check",
    description: "Verify all systems are healthy every 15 minutes",
    type: "shell",
    command: "curl -sf http://localhost:3000/health || echo 'ALERT: Health check failed'",
    expression: "*/15 * * * *",
    icon: "health",
  },
  {
    name: "Database Backup",
    description: "Backup the database every night at midnight",
    type: "shell",
    command: "sqlite3 ~/.seaclaw/memory.db '.backup ~/.seaclaw/backups/memory-$(date +%Y%m%d).db'",
    expression: "0 0 * * *",
    icon: "backup",
  },
];

function isValidCron(expr: string): boolean {
  const parts = expr.trim().split(/\s+/);
  return parts.length >= 5;
}

function cronToHuman(expr: string): string {
  const parts = expr.trim().split(/\s+/);
  if (parts.length < 5) return expr;
  const [min, hour, dom, month, dow] = parts;
  const every = (f: string, unit: string) =>
    f === "*" ? `Every ${unit}` : f.startsWith("*/") ? `Every ${f.slice(2)} ${unit}s` : null;
  if (min === "*" && hour === "*" && dom === "*" && month === "*" && dow === "*")
    return "Every minute";
  if (every(min, "minute")) return every(min, "minute")!;
  if (every(hour, "hour") && min === "0") return every(hour, "hour")!;
  if (hour !== "*" && !hour.startsWith("*/") && min === "0") {
    const h = parseInt(hour, 10);
    const ampm = h >= 12 ? (h === 12 ? "noon" : `${h - 12}pm`) : h === 0 ? "midnight" : `${h}am`;
    if (dom === "*" && month === "*" && dow === "*") return `Daily at ${ampm}`;
    if (dow !== "*" && dom === "*" && month === "*") {
      const dayLabel = dow === "1-5" ? "Weekdays" : dow === "0,6" ? "Weekends" : `Days ${dow}`;
      return `${dayLabel} at ${ampm}`;
    }
  }
  if (dom !== "*" && month === "*" && hour === "0" && min === "0") return `Monthly on day ${dom}`;
  return expr;
}

@customElement("sc-automation-form")
export class ScAutomationForm extends LitElement {
  @property({ type: String }) type: "agent" | "shell" = "agent";
  @property({ type: Object }) template?: AutomationTemplate | null;
  @property({ type: Object }) editingJob?: CronJob | null;
  @property({ type: Array }) channels: ChannelStatus[] = [];

  @state() private name = "";
  @state() private prompt = "";
  @state() private command = "";
  @state() private channel = "";
  @state() private schedule = "0 8 * * *";
  @state() private oneShot = false;
  @state() private formError = "";

  static override styles = css`
    :host {
      display: block;
      font-family: var(--sc-font);
      color: var(--sc-text);
    }

    .form-group {
      margin-bottom: var(--sc-space-md);
    }

    .form-group label {
      display: block;
      font-size: var(--sc-text-sm);
      font-weight: var(--sc-weight-medium);
      color: var(--sc-text-muted);
      margin-bottom: var(--sc-space-xs);
    }

    sc-textarea {
      width: 100%;
    }

    .form-select {
      width: 100%;
      box-sizing: border-box;
      padding: var(--sc-space-sm) var(--sc-space-md);
      font-family: var(--sc-font);
      font-size: var(--sc-text-base);
      background: var(--sc-bg-elevated);
      border: 1px solid var(--sc-border);
      border-radius: var(--sc-radius);
      color: var(--sc-text);
      cursor: pointer;
      transition:
        border-color var(--sc-duration-fast) var(--sc-ease-out),
        background var(--sc-duration-fast) var(--sc-ease-out);
    }

    .form-select:hover:not(:disabled) {
      border-color: var(--sc-text-faint);
    }

    .form-select:focus-visible {
      outline: 2px solid var(--sc-accent);
      outline-offset: var(--sc-focus-ring-offset);
    }

    .form-error {
      font-size: var(--sc-text-sm);
      color: var(--sc-error);
      margin-top: var(--sc-space-xs);
    }

    .modal-footer {
      display: flex;
      justify-content: flex-end;
      gap: var(--sc-space-sm);
      margin-top: var(--sc-space-xl);
      padding-top: var(--sc-space-lg);
      border-top: 1px solid var(--sc-border);
    }

    .mode-toggle {
      display: flex;
      gap: var(--sc-space-xs);
    }

    .run-once-message {
      font-size: var(--sc-text-sm);
      color: var(--sc-text-muted);
      padding: var(--sc-space-sm);
      background: var(--sc-bg-elevated);
      border-radius: var(--sc-radius);
    }

    .mono-input :host(sc-input) input,
    .mono-input input {
      font-family: var(--sc-font-mono);
    }

    @media (prefers-reduced-motion: reduce) {
      .form-select {
        transition: none;
      }
    }
  `;

  override willUpdate(changed: Map<string, unknown>): void {
    if (changed.has("template") && this.template) {
      this._applyTemplate(this.template);
    } else if (changed.has("editingJob") && this.editingJob) {
      this._applyEditingJob(this.editingJob);
    } else if (
      (changed.has("template") || changed.has("editingJob")) &&
      !this.template &&
      !this.editingJob
    ) {
      this._resetForm();
    }
    if (changed.has("type") && !this.template && !this.editingJob) {
      this._resetForm();
    }
  }

  private _applyTemplate(t: AutomationTemplate): void {
    this.formError = "";
    if (t.type === "agent") {
      this.name = t.name;
      this.prompt = t.prompt ?? "";
      this.channel = (t as { channel?: string }).channel ?? "";
      this.schedule = t.expression;
      this.oneShot = false;
    } else {
      this.name = t.name;
      this.command = t.command ?? "";
      this.schedule = t.expression;
      this.oneShot = false;
    }
  }

  private _applyEditingJob(job: CronJob): void {
    this.formError = "";
    this.name = job.name ?? "";
    this.schedule = job.expression ?? "0 8 * * *";
    this.oneShot = job.one_shot ?? false;
    if (job.type === "agent") {
      this.prompt = job.command ?? "";
      this.channel = job.channel ?? "";
    } else {
      this.command = job.command ?? "";
    }
  }

  private _resetForm(): void {
    this.name = "";
    this.prompt = "";
    this.command = "";
    this.channel = "";
    this.schedule = "0 8 * * *";
    this.oneShot = false;
    this.formError = "";
  }

  private _dispatchSubmit(data: AutomationFormData): void {
    this.dispatchEvent(
      new CustomEvent("sc-automation-submit", {
        detail: data,
        bubbles: true,
        composed: true,
      }),
    );
  }

  private _dispatchCancel(): void {
    this.dispatchEvent(
      new CustomEvent("sc-automation-cancel", {
        bubbles: true,
        composed: true,
      }),
    );
  }

  private _validateAndSubmit(): void {
    const isAgent = this.type === "agent";
    const name = this.name.trim();
    const schedule = this.oneShot ? "0 0 1 1 *" : this.schedule.trim();

    if (!name) {
      this.formError = "Name is required";
      return;
    }
    if (isAgent) {
      const prompt = this.prompt.trim();
      if (!prompt) {
        this.formError = "Prompt is required";
        return;
      }
    } else {
      const cmd = this.command.trim();
      if (!cmd) {
        this.formError = "Command is required";
        return;
      }
    }
    if (!this.oneShot && !schedule) {
      this.formError = "Schedule is required";
      return;
    }
    if (!this.oneShot && !isValidCron(schedule)) {
      this.formError = "Invalid cron expression (need 5 parts: min hour day month weekday)";
      return;
    }

    this.formError = "";
    const data: AutomationFormData = {
      type: this.type,
      name: name || (isAgent ? "Agent task" : "Shell job"),
      expression: schedule,
      one_shot: this.oneShot,
    };
    if (isAgent) {
      data.prompt = this.prompt.trim();
      data.channel = this.channel || undefined;
    } else {
      data.command = this.command.trim();
    }
    this._dispatchSubmit(data);
  }

  private _renderAgentFields() {
    return html`
      <div class="form-group">
        <sc-input
          label="Name"
          placeholder="My daily summary"
          .value=${this.name}
          @sc-input=${(e: CustomEvent<{ value: string }>) => (this.name = e.detail.value)}
        ></sc-input>
      </div>
      <div class="form-group">
        <label for="agent-prompt">Prompt</label>
        <sc-textarea
          id="agent-prompt"
          placeholder="Summarize my unread messages..."
          .value=${this.prompt}
          rows="3"
          @sc-input=${(e: CustomEvent<{ value: string }>) => (this.prompt = e.detail.value)}
        ></sc-textarea>
      </div>
      <div class="form-group">
        <label>Mode</label>
        <div class="mode-toggle">
          <sc-button
            variant=${!this.oneShot ? "primary" : "secondary"}
            @click=${() => (this.oneShot = false)}
          >
            Recurring
          </sc-button>
          <sc-button
            variant=${this.oneShot ? "primary" : "secondary"}
            @click=${() => (this.oneShot = true)}
          >
            Run Once
          </sc-button>
        </div>
      </div>
      ${this.oneShot
        ? html`
            <div class="form-group">
              <div class="run-once-message">Run immediately on save</div>
            </div>
          `
        : html`
            <div class="form-group">
              <label>Schedule</label>
              <sc-schedule-builder
                .value=${this.schedule}
                @sc-schedule-change=${(e: CustomEvent<{ value: string }>) =>
                  (this.schedule = e.detail.value)}
              ></sc-schedule-builder>
            </div>
          `}
      <div class="form-group">
        <label for="agent-channel">Channel</label>
        <select
          id="agent-channel"
          class="form-select"
          .value=${this.channel}
          @change=${(e: Event) => (this.channel = (e.target as HTMLSelectElement).value)}
        >
          <option value="">— Gateway (default) —</option>
          ${this.channels.map(
            (ch) => html`<option value=${ch.key} ?disabled=${!ch.configured}>${ch.name}</option>`,
          )}
        </select>
      </div>
    `;
  }

  private _renderShellFields() {
    return html`
      <div class="form-group">
        <sc-input
          label="Name"
          placeholder="My backup script"
          .value=${this.name}
          @sc-input=${(e: CustomEvent<{ value: string }>) => (this.name = e.detail.value)}
        ></sc-input>
      </div>
      <div class="form-group mono-input">
        <sc-input
          label="Command"
          placeholder="echo 'hello world'"
          .value=${this.command}
          @sc-input=${(e: CustomEvent<{ value: string }>) => (this.command = e.detail.value)}
        ></sc-input>
      </div>
      <div class="form-group">
        <label>Mode</label>
        <div class="mode-toggle">
          <sc-button
            variant=${!this.oneShot ? "primary" : "secondary"}
            @click=${() => (this.oneShot = false)}
          >
            Recurring
          </sc-button>
          <sc-button
            variant=${this.oneShot ? "primary" : "secondary"}
            @click=${() => (this.oneShot = true)}
          >
            Run Once
          </sc-button>
        </div>
      </div>
      ${this.oneShot
        ? html`
            <div class="form-group">
              <div class="run-once-message">Run immediately on save</div>
            </div>
          `
        : html`
            <div class="form-group">
              <label>Schedule</label>
              <sc-schedule-builder
                .value=${this.schedule}
                @sc-schedule-change=${(e: CustomEvent<{ value: string }>) =>
                  (this.schedule = e.detail.value)}
              ></sc-schedule-builder>
            </div>
          `}
    `;
  }

  override render() {
    const isAgent = this.type === "agent";
    const isEditing = !!this.editingJob;
    const formTitle = isAgent ? "Job details" : "Job details";
    const formDesc = isAgent ? "Name, prompt, and schedule" : "Name, command, and schedule";

    return html`
      <sc-form-group title=${formTitle} description=${formDesc}>
        ${isAgent ? this._renderAgentFields() : this._renderShellFields()}
        ${this.formError ? html`<p class="form-error">${this.formError}</p>` : nothing}
      </sc-form-group>
      <div class="modal-footer">
        <sc-button variant="secondary" @click=${this._dispatchCancel}>Cancel</sc-button>
        <sc-button variant="primary" @click=${this._validateAndSubmit}>
          ${isEditing ? "Save" : "Create"}
        </sc-button>
      </div>
    `;
  }
}

export { cronToHuman };

declare global {
  interface HTMLElementTagNameMap {
    "sc-automation-form": ScAutomationForm;
  }
}
