import { LitElement, html, css } from "lit";
import { customElement, property } from "lit/decorators.js";
import "../components/sc-stat-card.js";
import "../components/sc-metric-row.js";

export interface StatMetric {
  label: string;
  value?: number;
  valueStr?: string;
}

export interface MetricRowItem {
  label: string;
  value: string;
  accent?: "success" | "error";
}

@customElement("sc-overview-stats")
export class ScOverviewStats extends LitElement {
  @property({ type: Array }) metrics: StatMetric[] = [];
  @property({ type: Array }) metricRowItems: MetricRowItem[] = [];

  static override styles = css`
    :host {
      display: block;
    }

    .metrics-block {
      margin-bottom: var(--sc-space-2xl);
    }

    .stats-row {
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(11.25rem, 1fr));
      gap: var(--sc-space-md);
      margin-bottom: var(--sc-space-2xl);
    }

    @media (max-width: 40rem) /* --sc-breakpoint-md */ {
      .stats-row {
        grid-template-columns: 1fr 1fr;
      }
    }

    @media (max-width: 30rem) /* --sc-breakpoint-sm */ {
      .stats-row {
        grid-template-columns: 1fr;
      }
    }
  `;

  override render() {
    return html`
      <div class="metrics-block">
        <div class="stats-row">
          ${this.metrics.map(
            (m, i) => html`
              <sc-stat-card
                .value=${m.value ?? 0}
                .valueStr=${m.valueStr ?? ""}
                .label=${m.label}
                style="--sc-stagger-delay: ${i * 50}ms"
              ></sc-stat-card>
            `,
          )}
        </div>
        <sc-metric-row .items=${this.metricRowItems}></sc-metric-row>
      </div>
    `;
  }
}

declare global {
  interface HTMLElementTagNameMap {
    "sc-overview-stats": ScOverviewStats;
  }
}
