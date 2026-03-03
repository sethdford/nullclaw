import { LitElement } from "lit";
import type { GatewayClient } from "./gateway.js";
import { getGateway } from "./gateway-provider.js";

/**
 * Base class for views that depend on gateway connectivity.
 * Calls `load()` automatically once the gateway is connected,
 * and re-calls it on reconnect.
 */
export class GatewayAwareLitElement extends LitElement {
  private _statusHandler = ((e: CustomEvent<string>) => {
    if (e.detail === "connected") this.load();
  }) as EventListener;

  protected get gateway(): GatewayClient | null {
    return getGateway();
  }

  override connectedCallback(): void {
    super.connectedCallback();
    const gw = this.gateway;
    if (gw) {
      gw.addEventListener("status", this._statusHandler);
      if (gw.status === "connected") {
        this.load();
      }
    }
  }

  override disconnectedCallback(): void {
    super.disconnectedCallback();
    this.gateway?.removeEventListener("status", this._statusHandler);
  }

  protected load(): void {}
}
