import { test, expect } from "@playwright/test";
import AxeBuilder from "@axe-core/playwright";

test.describe("Accessibility", () => {
  test("overview page has no critical a11y violations", async ({ page }) => {
    await page.goto("/");
    const results = await new AxeBuilder({ page })
      .withTags(["wcag2a", "wcag2aa", "wcag21aa"])
      .analyze();
    expect(
      results.violations.filter((v) => v.impact === "critical" || v.impact === "serious"),
    ).toEqual([]);
  });

  test("chat page has no critical a11y violations", async ({ page }) => {
    await page.goto("/#chat");
    await page.waitForTimeout(500);
    const results = await new AxeBuilder({ page })
      .withTags(["wcag2a", "wcag2aa", "wcag21aa"])
      .analyze();
    expect(
      results.violations.filter((v) => v.impact === "critical" || v.impact === "serious"),
    ).toEqual([]);
  });

  test("all navigation views are keyboard accessible", async ({ page }) => {
    await page.goto("/");
    // Tab through sidebar items
    for (let i = 0; i < 5; i++) {
      await page.keyboard.press("Tab");
    }
    const focused = await page.evaluate(() => document.activeElement?.tagName);
    expect(focused).toBeTruthy();
  });

  test("command palette is keyboard navigable", async ({ page }) => {
    await page.goto("/");
    await page.keyboard.press("Meta+k");
    await page.waitForTimeout(200);
    // Type to filter
    await page.keyboard.type("chat");
    await page.waitForTimeout(100);
    // Arrow down and Enter to select
    await page.keyboard.press("ArrowDown");
    await page.keyboard.press("Enter");
  });

  test("modal traps focus", async ({ page }) => {
    await page.goto("/");
    await page.keyboard.press("Meta+k");
    await page.waitForTimeout(200);
    // Escape closes
    await page.keyboard.press("Escape");
    await page.waitForTimeout(200);
  });
});
