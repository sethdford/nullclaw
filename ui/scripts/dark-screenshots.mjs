#!/usr/bin/env node
/**
 * Takes dark mode screenshots of the chat page and overview.
 * Run: node scripts/dark-screenshots.mjs
 * Requires: dev server on http://localhost:5173
 */
import { chromium } from 'playwright';

const BASE = 'http://localhost:5173';
const WAIT_MS = 2000;

async function main() {
  const browser = await chromium.launch();
  const context = await browser.newContext({
    colorScheme: 'dark',
  });
  const page = await context.newPage();

  const shots = [
    { url: `${BASE}/#chat`, path: '/tmp/dark-chat.png' },
    { url: `${BASE}/#chat:dark-test`, path: '/tmp/dark-welcome.png' },
    { url: `${BASE}/#overview`, path: '/tmp/dark-overview.png' },
  ];

  for (const { url, path } of shots) {
    await page.goto(url);
    await page.waitForTimeout(WAIT_MS);
    await page.screenshot({ path });
    console.log(`Saved: ${path}`);
  }

  await browser.close();
  console.log('\nScreenshots saved to:');
  shots.forEach(({ path }) => console.log(`  ${path}`));
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
