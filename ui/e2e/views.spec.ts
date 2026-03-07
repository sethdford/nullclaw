import { test, expect } from "@playwright/test";

test.describe("Secondary Views", () => {
  test("config view renders", async ({ page }) => {
    await page.goto("/#config");
    await page.waitForTimeout(500);
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
    const configView = app.locator("sc-config-view");
    await expect(configView).toBeAttached({ timeout: 5000 });
  });

  test("models view renders", async ({ page }) => {
    await page.goto("/#models");
    await page.waitForTimeout(500);
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
    const modelsView = app.locator("sc-models-view");
    await expect(modelsView).toBeAttached({ timeout: 5000 });
  });

  test("tools view renders", async ({ page }) => {
    await page.goto("/#tools");
    await page.waitForTimeout(500);
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
    const toolsView = app.locator("sc-tools-view");
    await expect(toolsView).toBeAttached({ timeout: 5000 });
  });

  test("sessions view renders", async ({ page }) => {
    await page.goto("/#sessions");
    await page.waitForTimeout(500);
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
    const sessionsView = app.locator("sc-sessions-view");
    await expect(sessionsView).toBeAttached({ timeout: 5000 });
  });

  test("nodes view renders", async ({ page }) => {
    await page.goto("/#nodes");
    await page.waitForTimeout(500);
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
    const nodesView = app.locator("sc-nodes-view");
    await expect(nodesView).toBeAttached({ timeout: 5000 });
  });

  test("navigation between views works", async ({ page }) => {
    await page.goto("/");
    await page.waitForTimeout(500);

    // Navigate to chat
    await page.goto("/#chat");
    await page.waitForTimeout(300);
    const chatView = page.locator("sc-app >> sc-chat-view");
    await expect(chatView).toBeAttached({ timeout: 5000 });

    // Navigate to config
    await page.goto("/#config");
    await page.waitForTimeout(300);
    const configView = page.locator("sc-app >> sc-config-view");
    await expect(configView).toBeAttached({ timeout: 5000 });
  });

  test("unknown hash falls back gracefully", async ({ page }) => {
    await page.goto("/#nonexistent");
    await page.waitForTimeout(500);
    // Should not crash, should show some view
    const app = page.locator("sc-app");
    await expect(app).toBeAttached({ timeout: 5000 });
  });
});
