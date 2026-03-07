import { html, css, nothing } from "lit";
import { customElement, state } from "lit/decorators.js";
import { GatewayAwareLitElement } from "../gateway-aware.js";
import { icons } from "../icons.js";
import "../components/sc-page-hero.js";
import "../components/sc-section-header.js";
import "../components/sc-stat-card.js";
import "../components/sc-card.js";
import "../components/sc-skeleton.js";
import "../components/sc-empty-state.js";
import "../components/sc-animated-number.js";
import "../components/sc-sparkline-enhanced.js";
import "../components/sc-metric-row.js";
import "../components/sc-forecast-chart.js";

interface DailyCost {
  date: string;
  cost: number;
}

interface ProviderUsage {
  provider: string;
  tokens: number;
  cost: number;
}

interface UsageSummary {
  session_cost_usd?: number;
  daily_cost_usd?: number;
  monthly_cost_usd?: number;
  projected_monthly_usd?: number;
  previous_month_cost_usd?: number;
  total_tokens?: number;
  request_count?: number;
  token_trend?: number[];
  daily_cost_history?: DailyCost[];
  by_provider?: ProviderUsage[];
  cost_per_request?: number;
  tokens_per_turn?: number;
  turns_today?: number;
  turns_week?: number;
  days_in_month?: number;
}

const CHART_COLORS = [
  "var(--sc-chart-categorical-1)",
  "var(--sc-chart-categorical-2)",
  "var(--sc-chart-categorical-3)",
  "var(--sc-chart-categorical-4)",
  "var(--sc-chart-categorical-5)",
];

@customElement("sc-usage-view")
export class ScUsageView extends GatewayAwareLitElement {
  static override styles = css`
    :host {
      display: block;
      color: var(--sc-text);
      max-width: 960px;
    }
    .stats-row {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      gap: var(--sc-space-md);
      margin-bottom: var(--sc-space-2xl);
    }
    @media (max-width: 768px) /* --sc-breakpoint-lg */ {
      .stats-row {
        grid-template-columns: repeat(2, 1fr);
      }
    }
    @media (max-width: 480px) /* --sc-breakpoint-sm */ {
      .stats-row {
        grid-template-columns: 1fr;
      }
    }
    .section {
      margin-bottom: var(--sc-space-2xl);
    }
    .section-label {
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
      text-transform: uppercase;
      letter-spacing: 0.06em;
      margin-bottom: var(--sc-space-sm);
    }
    .chart-inner {
      padding: var(--sc-space-sm) var(--sc-space-md) var(--sc-space-md);
    }
    .provider-list {
      display: flex;
      flex-direction: column;
      gap: var(--sc-space-md);
    }
    .provider-row {
      display: flex;
      align-items: center;
      gap: var(--sc-space-sm);
    }
    .provider-dot {
      width: 10px;
      height: 10px;
      border-radius: var(--sc-radius-full);
      flex-shrink: 0;
    }
    .provider-name {
      width: 6.5rem;
      font-size: var(--sc-text-sm);
      color: var(--sc-text);
      flex-shrink: 0;
      font-weight: var(--sc-weight-medium);
    }
    .provider-bar-track {
      flex: 1;
      height: var(--sc-space-md, 12px);
      background: var(--sc-bg-inset);
      border-radius: var(--sc-radius-sm);
      overflow: hidden;
    }
    .provider-bar-fill {
      height: 100%;
      border-radius: var(--sc-radius-sm);
      transition: width var(--sc-duration-normal) var(--sc-ease-out);
    }
    .provider-cost {
      font-size: var(--sc-text-sm);
      color: var(--sc-text);
      font-variant-numeric: tabular-nums;
      font-weight: var(--sc-weight-medium);
      min-width: 4rem;
      text-align: right;
    }
    .provider-pct {
      font-size: var(--sc-text-xs);
      color: var(--sc-text-muted);
      min-width: 2.5rem;
      text-align: right;
    }
    @media (max-width: 480px) /* --sc-breakpoint-sm */ {
      .provider-name {
        width: 5rem;
      }
    }
    @media (prefers-reduced-motion: reduce) {
      .provider-bar-fill {
        transition: none;
      }
    }
  `;

  @state() private summary: UsageSummary = {};
  @state() private loading = false;
  @state() private error = "";

  protected override async load(): Promise<void> {
    await this.loadSummary();
  }

  private async loadSummary(): Promise<void> {
    const gw = this.gateway;
    if (!gw) return;
    this.loading = true;
    this.error = "";
    try {
      const res = (await gw.request<UsageSummary>("usage.summary", {})) as
        | UsageSummary
        | { result?: UsageSummary };
      this.summary =
        (res && "result" in res && res.result) ||
        (res && "session_cost_usd" in res ? (res as UsageSummary) : {}) ||
        {};
    } catch (e) {
      this.error = e instanceof Error ? e.message : "Failed to load usage";
      this.summary = {};
    } finally {
      this.loading = false;
    }
  }

  private formatCurrency(v: number | undefined): string {
    if (v == null || Number.isNaN(v)) return "$0.00";
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
      minimumFractionDigits: 2,
    }).format(v);
  }

  private formatNumber(v: number | undefined): string {
    if (v == null || Number.isNaN(v)) return "0";
    return new Intl.NumberFormat("en-US").format(v);
  }

  private _trendBadge(): { trend: string; direction: "up" | "down" | "flat" } {
    const projected = this.summary.projected_monthly_usd ?? 0;
    const previous = this.summary.previous_month_cost_usd ?? 0;
    if (previous <= 0) return { trend: "", direction: "flat" };
    const pctChange = ((projected - previous) / previous) * 100;
    const sign = pctChange >= 0 ? "+" : "";
    return {
      trend: `${sign}${Math.round(pctChange)}% vs last mo`,
      direction: pctChange > 0 ? "up" : pctChange < 0 ? "down" : "flat",
    };
  }

  private _renderSkeleton() {
    return html`
      <div class="stats-row sc-stagger">
        <sc-skeleton variant="card" height="90px"></sc-skeleton>
        <sc-skeleton variant="card" height="90px"></sc-skeleton>
        <sc-skeleton variant="card" height="90px"></sc-skeleton>
        <sc-skeleton variant="card" height="90px"></sc-skeleton>
      </div>
      <sc-skeleton variant="card" height="200px"></sc-skeleton>
    `;
  }

  private _renderForecast() {
    const history = this.summary.daily_cost_history;
    if (!history || history.length < 2) return nothing;

    return html`
      <div class="section">
        <div class="section-label">Monthly Forecast</div>
        <sc-card glass>
          <div class="chart-inner">
            <sc-forecast-chart
              .history=${history}
              .projectedTotal=${this.summary.projected_monthly_usd ?? 0}
              .daysInMonth=${this.summary.days_in_month ?? 31}
            ></sc-forecast-chart>
          </div>
        </sc-card>
      </div>
    `;
  }

  private _renderTokenTrend() {
    const trend = this.summary.token_trend;
    if (!trend || trend.length < 2) return nothing;
    return html`
      <div class="section">
        <div class="section-label">Token Usage (24h)</div>
        <sc-card glass>
          <div class="chart-inner">
            <sc-sparkline-enhanced
              .data=${trend}
              .width=${640}
              .height=${64}
              tooltipLabel="tokens"
            ></sc-sparkline-enhanced>
          </div>
        </sc-card>
      </div>
    `;
  }

  private _renderProviders() {
    const providers = this.summary.by_provider;
    if (!providers || providers.length === 0) return nothing;

    const totalCost = providers.reduce((s, p) => s + p.cost, 0) || 1;
    const maxCost = Math.max(...providers.map((p) => p.cost), 0.01);

    return html`
      <div class="section">
        <div class="section-label">Cost by Provider</div>
        <sc-card glass>
          <div class="provider-list">
            ${providers.map(
              (p, i) => html`
                <div class="provider-row">
                  <span
                    class="provider-dot"
                    style="background: ${CHART_COLORS[i % CHART_COLORS.length]}"
                  ></span>
                  <span class="provider-name">${p.provider}</span>
                  <div class="provider-bar-track">
                    <div
                      class="provider-bar-fill"
                      style="width: ${(p.cost / maxCost) * 100}%; background: ${CHART_COLORS[
                        i % CHART_COLORS.length
                      ]}"
                    ></div>
                  </div>
                  <span class="provider-cost">${this.formatCurrency(p.cost)}</span>
                  <span class="provider-pct">${Math.round((p.cost / totalCost) * 100)}%</span>
                </div>
              `,
            )}
          </div>
        </sc-card>
      </div>
    `;
  }

  private _renderMetrics() {
    const s = this.summary;
    const items = [
      { label: "Avg Cost / Request", value: this.formatCurrency(s.cost_per_request) },
      { label: "Tokens / Turn", value: this.formatNumber(s.tokens_per_turn) },
      { label: "Turns Today", value: this.formatNumber(s.turns_today) },
      { label: "This Week", value: this.formatNumber(s.turns_week) },
    ];
    return html`<sc-metric-row .items=${items}></sc-metric-row>`;
  }

  override render() {
    const s = this.summary;
    const dailyCost = s.daily_cost_usd ?? 0;
    const monthlyCost = s.monthly_cost_usd ?? 0;
    const projectedCost = s.projected_monthly_usd ?? 0;
    const requestCount = s.request_count ?? 0;
    const totalTokens = s.total_tokens ?? 0;
    const { trend, direction } = this._trendBadge();

    const isEmpty =
      dailyCost === 0 &&
      monthlyCost === 0 &&
      projectedCost === 0 &&
      totalTokens === 0 &&
      requestCount === 0;

    return html`
      <sc-page-hero>
        <sc-section-header
          heading="Usage"
          description="Token consumption, cost tracking, and forecasting"
        ></sc-section-header>
      </sc-page-hero>

      <div class="stats-row">
        <sc-stat-card
          .value=${dailyCost}
          label="Spend Today"
          prefix="$"
          style="--sc-stagger-delay: 0ms"
        ></sc-stat-card>
        <sc-stat-card
          .value=${monthlyCost}
          label="This Month"
          prefix="$"
          style="--sc-stagger-delay: 80ms"
        ></sc-stat-card>
        <sc-stat-card
          .value=${projectedCost}
          label="Projected Month-End"
          prefix="$"
          trend=${trend}
          trendDirection=${direction}
          style="--sc-stagger-delay: 160ms"
        ></sc-stat-card>
        <sc-stat-card
          .value=${requestCount}
          label="Requests"
          style="--sc-stagger-delay: 240ms"
        ></sc-stat-card>
      </div>

      ${this.error
        ? html`<sc-empty-state
            .icon=${icons.warning}
            heading="Error"
            description=${this.error}
          ></sc-empty-state>`
        : nothing}
      ${this.loading
        ? this._renderSkeleton()
        : isEmpty
          ? html`
              <sc-empty-state
                .icon=${icons["bar-chart"]}
                heading="No usage data"
                description="Usage metrics will appear here once you start making requests."
              ></sc-empty-state>
            `
          : html`
              ${this._renderForecast()} ${this._renderTokenTrend()} ${this._renderProviders()}
              ${this._renderMetrics()}
            `}
    `;
  }
}
