import { describe, it, expect, vi, beforeEach, afterEach } from "vitest";
import {
  DragDismissController,
  SwipeController,
  rippleEffect,
  prefersReducedMotion,
  PullRefreshController,
} from "./gesture.js";

describe("prefersReducedMotion", () => {
  const originalMatchMedia = window.matchMedia;

  afterEach(() => {
    window.matchMedia = originalMatchMedia;
  });

  it("returns false when prefers-reduced-motion is not set", () => {
    window.matchMedia = vi.fn().mockImplementation((query: string) => ({
      matches: false,
      media: query,
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    }));
    expect(prefersReducedMotion()).toBe(false);
  });

  it("returns true when prefers-reduced-motion: reduce", () => {
    window.matchMedia = vi.fn().mockImplementation((query: string) => ({
      matches: query === "(prefers-reduced-motion: reduce)",
      media: query,
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    }));
    expect(prefersReducedMotion()).toBe(true);
  });
});

describe("DragDismissController", () => {
  it("initializes without error when added to host", () => {
    const host = document.createElement("div");
    const addController = vi.fn();
    (host as unknown as { addController: (c: unknown) => void }).addController = addController;

    expect(() => new DragDismissController(host as never)).not.toThrow();
    expect(addController).toHaveBeenCalled();
  });
});

describe("SwipeController", () => {
  it("accepts threshold configuration via options", () => {
    const host = document.createElement("div");
    const addController = vi.fn();
    (host as unknown as { addController: (c: unknown) => void }).addController = addController;

    const controller = new SwipeController(host as never, {
      minDistance: 75,
      velocityThreshold: 0.5,
    });

    expect(controller).toBeDefined();
    expect(addController).toHaveBeenCalled();
  });

  it("uses defaults when no options provided", () => {
    const host = document.createElement("div");
    const addController = vi.fn();
    (host as unknown as { addController: (c: unknown) => void }).addController = addController;

    expect(() => new SwipeController(host as never)).not.toThrow();
  });
});

describe("rippleEffect", () => {
  beforeEach(() => {
    vi.spyOn(globalThis, "matchMedia").mockReturnValue({
      matches: false,
      media: "",
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    } as unknown as MediaQueryList);
  });

  afterEach(() => {
    vi.restoreAllMocks();
  });

  it("does not throw on a mock element with pointer event", () => {
    const element = document.createElement("div");
    element.getBoundingClientRect = () => ({
      left: 0,
      top: 0,
      right: 100,
      bottom: 100,
      width: 100,
      height: 100,
      x: 0,
      y: 0,
      toJSON: () => ({}),
    });
    document.body.appendChild(element);

    const event = new PointerEvent("pointerdown", { clientX: 50, clientY: 50 });

    expect(() => rippleEffect(element, event)).not.toThrow();

    document.body.removeChild(element);
  });

  it("does nothing when prefers-reduced-motion is true", () => {
    vi.mocked(matchMedia).mockReturnValue({
      matches: true,
      media: "(prefers-reduced-motion: reduce)",
      onchange: null,
      addListener: vi.fn(),
      removeListener: vi.fn(),
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      dispatchEvent: vi.fn(),
    } as unknown as MediaQueryList);

    const element = document.createElement("div");
    element.getBoundingClientRect = () => ({
      left: 0,
      top: 0,
      right: 100,
      bottom: 100,
      width: 100,
      height: 100,
      x: 0,
      y: 0,
      toJSON: () => ({}),
    });

    const event = new PointerEvent("pointerdown", { clientX: 50, clientY: 50 });

    rippleEffect(element, event);

    expect(element.children.length).toBe(0);
  });
});

describe("PullRefreshController", () => {
  it("initializes without error when given host and scroll target", () => {
    const host = document.createElement("div");
    const scrollTarget = document.createElement("div");
    scrollTarget.style.overflow = "auto";
    host.appendChild(scrollTarget);

    const addController = vi.fn();
    (host as unknown as { addController: (c: unknown) => void }).addController = addController;

    expect(() => new PullRefreshController(host as never, scrollTarget)).not.toThrow();
    expect(addController).toHaveBeenCalled();
  });
});
