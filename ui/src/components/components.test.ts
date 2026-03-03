import { describe, it, expect } from "vitest";

describe("sc-button", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-button.js");
    expect(customElements.get("sc-button")).toBeDefined();
  });

  it("should reflect variant property", async () => {
    const { ScButton } = await import("./sc-button.js");
    const el = new ScButton();
    el.variant = "primary";
    expect(el.variant).toBe("primary");
  });

  it("should reflect disabled property", async () => {
    const { ScButton } = await import("./sc-button.js");
    const el = new ScButton();
    el.disabled = true;
    expect(el.disabled).toBe(true);
  });
});

describe("sc-card", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-card.js");
    expect(customElements.get("sc-card")).toBeDefined();
  });
});

describe("sc-badge", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-badge.js");
    expect(customElements.get("sc-badge")).toBeDefined();
  });

  it("should reflect variant property", async () => {
    const { ScBadge } = await import("./sc-badge.js");
    const el = new ScBadge();
    el.variant = "success";
    expect(el.variant).toBe("success");
  });
});

describe("sc-modal", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-modal.js");
    expect(customElements.get("sc-modal")).toBeDefined();
  });

  it("should default to closed", async () => {
    const { ScModal } = await import("./sc-modal.js");
    const el = new ScModal();
    expect(el.open).toBe(false);
  });
});

describe("sc-tooltip", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-tooltip.js");
    expect(customElements.get("sc-tooltip")).toBeDefined();
  });
});

describe("sc-empty-state", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-empty-state.js");
    expect(customElements.get("sc-empty-state")).toBeDefined();
  });
});

describe("sc-skeleton", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-skeleton.js");
    expect(customElements.get("sc-skeleton")).toBeDefined();
  });
});

describe("sc-sheet", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-sheet.js");
    expect(customElements.get("sc-sheet")).toBeDefined();
  });

  it("should default to closed", async () => {
    const { ScSheet } = await import("./sc-sheet.js");
    const el = new ScSheet();
    expect(el.open).toBe(false);
  });
});

describe("sc-toast", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-toast.js");
    expect(customElements.get("sc-toast")).toBeDefined();
  });
});

describe("sc-tabs", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-tabs.js");
    expect(customElements.get("sc-tabs")).toBeDefined();
  });

  it("should accept tabs array", async () => {
    const { ScTabs } = await import("./sc-tabs.js");
    const el = new ScTabs();
    el.tabs = [
      { id: "a", label: "A" },
      { id: "b", label: "B" },
    ];
    expect(el.tabs).toHaveLength(2);
  });
});

describe("sc-avatar", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-avatar.js");
    expect(customElements.get("sc-avatar")).toBeDefined();
  });

  it("should store name property", async () => {
    const { ScAvatar } = await import("./sc-avatar.js");
    const el = new ScAvatar();
    el.name = "John Doe";
    expect(el.name).toBe("John Doe");
  });
});

describe("sc-progress", () => {
  it("should be defined as a custom element", async () => {
    await import("./sc-progress.js");
    expect(customElements.get("sc-progress")).toBeDefined();
  });

  it("should default to 0 value", async () => {
    const { ScProgress } = await import("./sc-progress.js");
    const el = new ScProgress();
    expect(el.value).toBe(0);
  });

  it("should accept indeterminate mode", async () => {
    const { ScProgress } = await import("./sc-progress.js");
    const el = new ScProgress();
    el.indeterminate = true;
    expect(el.indeterminate).toBe(true);
  });
});
