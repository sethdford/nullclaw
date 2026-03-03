#!/usr/bin/env node
/**
 * Design tokens build pipeline
 * Reads W3C-format token JSON files and generates platform-specific outputs.
 * Run: npx tsx design-tokens/build.ts
 */

import * as fs from "fs";
import * as path from "path";
import { fileURLToPath } from "url";

const REM_PX = 16;
const SCRIPT_DIR = path.dirname(fileURLToPath(import.meta.url));
const ROOT = path.resolve(SCRIPT_DIR, "..");
const TOKENS_DIR = path.join(ROOT, "design-tokens");

const TOKEN_FILES = [
  "base.tokens.json",
  "typography.tokens.json",
  "motion.tokens.json",
  "semantic.tokens.json",
  "components.tokens.json",
];

type TokenValue = string | number;
type TokenMap = Record<string, TokenValue>;

/** Recursively collect all $value entries into a flat path -> value map */
function collectTokens(obj: unknown, prefix = ""): TokenMap {
  const result: TokenMap = {};
  if (obj === null || typeof obj !== "object") return result;
  const rec = obj as Record<string, unknown>;

  for (const [key, val] of Object.entries(rec)) {
    if (key.startsWith("$")) continue;
    const pathPart = prefix ? `${prefix}.${key}` : key;
    if (val !== null && typeof val === "object" && "$value" in val) {
      const v = (val as { $value: TokenValue }).$value;
      result[pathPart] = v;
    } else if (typeof val === "object" && val !== null) {
      Object.assign(result, collectTokens(val, pathPart));
    }
  }
  return result;
}

/** Resolve {path.to.token} references in place; repeat until stable */
function resolveRefs(tokens: TokenMap): TokenMap {
  const resolved = { ...tokens };
  let changed = true;
  while (changed) {
    changed = false;
    for (const [key, val] of Object.entries(resolved)) {
      if (typeof val !== "string") continue;
      const ref = val.match(/^\{([^}]+)\}$/);
      if (ref) {
        const target = resolved[ref[1]];
        if (target !== undefined) {
          resolved[key] = target;
          changed = true;
        }
      }
    }
  }
  return resolved;
}

/** Convert rem to px (1rem = 16px). Returns number for px, string unchanged if not rem. */
function remToPx(val: string): number | null {
  const m = val.match(/^([\d.]+)rem$/);
  if (!m) return null;
  return Math.round(parseFloat(m[1]) * REM_PX);
}

/** Parse dimension (rem, px) to numeric px */
function dimToPx(val: string): number | null {
  if (val.endsWith("rem")) return remToPx(val);
  const m = val.match(/^(\d+)px$/);
  return m ? parseInt(m[1], 10) : null;
}

/** Convert hex color #rrggbb to 0xRRGGBB for Swift */
function hexToSwift(hex: string): string {
  const m = hex.match(/^#([0-9a-fA-F]{6})$/);
  if (!m) return "0x000000";
  return "0x" + m[1].toUpperCase();
}

/** Convert hex color to Kotlin Color(0xFFRRGGBB) */
function hexToKotlin(hex: string): string {
  const m = hex.match(/^#([0-9a-fA-F]{6})$/);
  if (!m) return "0xFF000000";
  return "0xFF" + m[1].toUpperCase();
}

/** Convert rgba(r,g,b,a) to Kotlin Color - approximate as opaque for simplicity */
function rgbaToKotlin(rgba: string): string {
  const m = rgba.match(/rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*([\d.]+))?\)/);
  if (!m) return "0xFF000000";
  const r = parseInt(m[1], 10);
  const g = parseInt(m[2], 10);
  const b = parseInt(m[3], 10);
  const a = m[4] ? Math.round(parseFloat(m[4]) * 255) : 255;
  const hex = ((a << 24) | (r << 16) | (g << 8) | b)
    .toString(16)
    .padStart(8, "0")
    .toUpperCase();
  return "0x" + hex;
}

function colorToSwift(val: string): string {
  if (val.startsWith("#")) return hexToSwift(val);
  if (val.startsWith("rgba")) return hexToSwift("#000000"); // Swift Color(hex:) doesn't support alpha directly; use placeholder
  return "0x000000";
}

function colorToKotlin(val: string): string {
  if (val.startsWith("#")) return hexToKotlin(val);
  if (val.startsWith("rgba")) return rgbaToKotlin(val);
  return "0xFF000000";
}

/** k=stiffness, c=damping, m=mass. SwiftUI: response ≈ 2π/√(k/m), dampingFraction = c/(2√(km)) */
function springToSwiftResponse(
  stiffness: number,
  damping: number,
  mass: number,
): number {
  return (
    Math.round(((2 * Math.PI) / Math.sqrt(stiffness / mass)) * 1000) / 1000
  );
}

function springToSwiftDampingFraction(
  stiffness: number,
  damping: number,
  mass: number,
): number {
  return (
    Math.round((damping / (2 * Math.sqrt(stiffness * mass))) * 1000) / 1000
  );
}

function main() {
  let tokens: TokenMap = {};
  for (const file of TOKEN_FILES) {
    const p = path.join(TOKENS_DIR, file);
    if (!fs.existsSync(p)) {
      console.error(`Missing token file: ${p}`);
      process.exit(1);
    }
    const data = JSON.parse(fs.readFileSync(p, "utf-8"));
    tokens = { ...tokens, ...collectTokens(data) };
  }
  tokens = resolveRefs(tokens);

  // ─── Output 1 & 2: CSS (ui + website) ───
  const css = generateCSS(tokens);
  const uiCssPath = path.join(ROOT, "ui", "src", "styles", "_tokens.css");
  const websiteCssPath = path.join(
    ROOT,
    "website",
    "src",
    "styles",
    "_tokens.css",
  );
  fs.mkdirSync(path.dirname(uiCssPath), { recursive: true });
  fs.mkdirSync(path.dirname(websiteCssPath), { recursive: true });
  fs.writeFileSync(uiCssPath, css, "utf-8");
  fs.writeFileSync(websiteCssPath, css, "utf-8");
  console.log("Wrote", uiCssPath);
  console.log("Wrote", websiteCssPath);

  // ─── Output 3: Swift ───
  const swift = generateSwift(tokens);
  const swiftPath = path.join(
    ROOT,
    "apps",
    "shared",
    "SeaClawKit",
    "Sources",
    "SeaClawChatUI",
    "DesignTokens.swift",
  );
  fs.mkdirSync(path.dirname(swiftPath), { recursive: true });
  fs.writeFileSync(swiftPath, swift, "utf-8");
  console.log("Wrote", swiftPath);

  // ─── Output 4: Kotlin ───
  const kotlin = generateKotlin(tokens);
  const kotlinPath = path.join(
    ROOT,
    "apps",
    "android",
    "app",
    "src",
    "main",
    "java",
    "ai",
    "seaclaw",
    "app",
    "ui",
    "DesignTokens.kt",
  );
  fs.mkdirSync(path.dirname(kotlinPath), { recursive: true });
  fs.writeFileSync(kotlinPath, kotlin, "utf-8");
  console.log("Wrote", kotlinPath);

  // ─── Output 5: C header ───
  const header = generateCHeader(tokens);
  const headerPath = path.join(ROOT, "include", "seaclaw", "design_tokens.h");
  fs.mkdirSync(path.dirname(headerPath), { recursive: true });
  fs.writeFileSync(headerPath, header, "utf-8");
  console.log("Wrote", headerPath);

  console.log("Done.");
}

function generateCSS(tokens: TokenMap): string {
  const lines: string[] = [
    "/* Auto-generated from design-tokens/ — do not edit manually */",
    ":root {",
  ];

  // Base: Spacing
  const spaceKeys = [
    "spacing.xs",
    "spacing.sm",
    "spacing.md",
    "spacing.lg",
    "spacing.xl",
    "spacing.2xl",
  ];
  lines.push("  /* Base: Spacing */");
  for (const k of spaceKeys) {
    const v = tokens[k];
    if (v != null)
      lines.push(`  --sc-space-${k.replace("spacing.", "")}: ${v};`);
  }

  // Base: Radius
  const radiusKeys = [
    "radius.sm",
    "radius.md",
    "radius.lg",
    "radius.xl",
    "radius.full",
  ];
  lines.push("  /* Base: Radius */");
  lines.push(`  --sc-radius: ${tokens["radius.md"] ?? "8px"};`);
  for (const k of radiusKeys) {
    const v = tokens[k];
    if (v != null)
      lines.push(`  --sc-radius-${k.replace("radius.", "")}: ${v};`);
  }

  // Typography
  lines.push("  /* Typography */");
  lines.push(
    `  --sc-font: ${tokens["fontFamily.sans"] ?? "'Avenir', sans-serif"};`,
  );
  lines.push(
    `  --sc-font-mono: ${tokens["fontFamily.mono"] ?? "'Geist Mono', monospace"};`,
  );
  for (const [k, v] of Object.entries(tokens)) {
    if (k.startsWith("fontSize.") && typeof v === "string")
      lines.push(`  --sc-text-${k.replace("fontSize.", "")}: ${v};`);
    if (k.startsWith("fontWeight.") && typeof v === "number")
      lines.push(`  --sc-weight-${k.replace("fontWeight.", "")}: ${v};`);
    if (k.startsWith("lineHeight.") && typeof v === "number")
      lines.push(`  --sc-leading-${k.replace("lineHeight.", "")}: ${v};`);
    if (k.startsWith("letterSpacing.") && typeof v === "string")
      lines.push(`  --sc-tracking-${k.replace("letterSpacing.", "")}: ${v};`);
  }

  // Motion
  lines.push("  /* Motion */");
  for (const [k, v] of Object.entries(tokens)) {
    if (k.startsWith("duration.") && typeof v === "string")
      lines.push(`  --sc-duration-${k.replace("duration.", "")}: ${v};`);
    if (k.startsWith("easing.") && typeof v === "string")
      lines.push(`  --sc-${k.replace("easing.", "")}: ${v};`);
    if (k.startsWith("transition.") && typeof v === "string")
      lines.push(`  --sc-transition: ${v};`);
  }
  for (const [k, v] of Object.entries(tokens)) {
    if (k.match(/^spring\.\w+\.stiffness$/)) {
      const name = k.split(".")[1];
      const stiff = v as number;
      const damp = (tokens[`spring.${name}.damping`] as number) ?? 20;
      const mass = (tokens[`spring.${name}.mass`] as number) ?? 1;
      lines.push(`  --sc-spring-${name}-stiffness: ${stiff};`);
      lines.push(`  --sc-spring-${name}-damping: ${damp};`);
    }
  }

  // Dark theme (default)
  lines.push("  /* Dark theme colors (default) */");
  const darkKeys = Object.keys(tokens).filter((k) => k.startsWith("dark."));
  for (const k of darkKeys.sort()) {
    const v = tokens[k];
    if (v == null) continue;
    const name = k.replace("dark.", "").replace(/-/g, "-");
    lines.push(`  --sc-${name}: ${v};`);
  }

  // Component tokens
  lines.push("  /* Component tokens */");
  const compKeys = ["sidebar.width", "sidebar.collapsed"];
  for (const k of compKeys) {
    const v = tokens[k];
    if (v != null) lines.push(`  --sc-${k.replace(".", "-")}: ${v};`);
  }

  lines.push("}");
  lines.push("");
  lines.push("@media (prefers-color-scheme: light) {");
  lines.push("  :root {");
  const lightKeys = Object.keys(tokens).filter((k) => k.startsWith("light."));
  for (const k of lightKeys.sort()) {
    const v = tokens[k];
    if (v == null) continue;
    const name = k.replace("light.", "").replace(/-/g, "-");
    lines.push(`    --sc-${name}: ${v};`);
  }
  lines.push("  }");
  lines.push("}");

  return lines.join("\n");
}

function generateSwift(tokens: TokenMap): string {
  const lines: string[] = [
    "// Auto-generated from design-tokens/ — do not edit manually",
    "import SwiftUI",
    "",
    "public enum SCTokens {",
  ];

  // Dark colors (exclude shadows - they're CSS values, not colors)
  lines.push("    // MARK: - Colors (Dark)");
  lines.push("    public enum Dark {");
  const darkKeys = Object.keys(tokens).filter(
    (k) =>
      k.startsWith("dark.") &&
      !k.includes("shadow") &&
      typeof tokens[k] === "string" &&
      ((tokens[k] as string).startsWith("#") ||
        (tokens[k] as string).startsWith("rgba")),
  );
  for (const k of darkKeys.sort()) {
    const v = tokens[k] as string;
    if (!v.startsWith("#") && !v.startsWith("rgba")) continue;
    const name = toSwiftCase(k.replace("dark.", ""));
    const colorExpr = formatSwiftColor(v);
    lines.push(`        public static let ${name} = ${colorExpr}`);
  }
  lines.push("    }");
  lines.push("");

  // Light colors (exclude shadows)
  lines.push("    // MARK: - Colors (Light)");
  lines.push("    public enum Light {");
  const lightKeys = Object.keys(tokens).filter(
    (k) =>
      k.startsWith("light.") &&
      !k.includes("shadow") &&
      typeof tokens[k] === "string" &&
      ((tokens[k] as string).startsWith("#") ||
        (tokens[k] as string).startsWith("rgba")),
  );
  for (const k of lightKeys.sort()) {
    const v = tokens[k] as string;
    if (!v.startsWith("#") && !v.startsWith("rgba")) continue;
    const name = toSwiftCase(k.replace("light.", ""));
    const colorExpr = formatSwiftColor(v);
    lines.push(`        public static let ${name} = ${colorExpr}`);
  }
  lines.push("    }");
  lines.push("");

  // Spacing (px)
  lines.push("    // MARK: - Spacing");
  const spaceMap: Record<string, string> = {
    "spacing.xs": "spaceXs",
    "spacing.sm": "spaceSm",
    "spacing.md": "spaceMd",
    "spacing.lg": "spaceLg",
    "spacing.xl": "spaceXl",
    "spacing.2xl": "space2xl",
  };
  for (const [path, name] of Object.entries(spaceMap)) {
    const v = tokens[path] as string | undefined;
    if (v) {
      const px = dimToPx(v);
      if (px != null)
        lines.push(`    public static let ${name}: CGFloat = ${px}`);
    }
  }
  lines.push("");

  // Radius
  lines.push("    // MARK: - Radius");
  const radiusMap: Record<string, string> = {
    "radius.sm": "radiusSm",
    "radius.md": "radiusMd",
    "radius.lg": "radiusLg",
    "radius.xl": "radiusXl",
  };
  for (const [path, name] of Object.entries(radiusMap)) {
    const v = tokens[path] as string | undefined;
    if (v) {
      const px = dimToPx(v);
      if (px != null)
        lines.push(`    public static let ${name}: CGFloat = ${px}`);
    }
  }
  lines.push("");

  // Typography
  lines.push("    // MARK: - Typography");
  const fontSans = (tokens["fontFamily.sans"] as string) ?? "Avenir";
  lines.push(
    `    public static let fontSans = "${fontSans.split(",")[0].replace(/^'\s*|\s*'$/g, "")}"`,
  );
  const fontMono = (tokens["fontFamily.mono"] as string) ?? "Geist Mono";
  lines.push(
    `    public static let fontMono = "${fontMono.split(",")[0].replace(/^'\s*|\s*'$/g, "")}"`,
  );
  lines.push("");

  // Spring
  lines.push("    // MARK: - Motion (Spring)");
  const springNames = ["micro", "standard", "expressive", "dramatic"];
  for (const name of springNames) {
    const stiff = tokens[`spring.${name}.stiffness`] as number;
    const damp = tokens[`spring.${name}.damping`] as number;
    const mass = (tokens[`spring.${name}.mass`] as number) ?? 1;
    if (stiff == null || damp == null) continue;
    const response = springToSwiftResponse(stiff, damp, mass);
    const df = springToSwiftDampingFraction(stiff, damp, mass);
    lines.push(
      `    public static let spring${name.charAt(0).toUpperCase() + name.slice(1)} = Animation.spring(response: ${response}, dampingFraction: ${df})`,
    );
  }
  lines.push("}");
  lines.push("");
  lines.push("extension Color {");
  lines.push("    init(hex: UInt, alpha: Double = 1) {");
  lines.push("        self.init(");
  lines.push("            .sRGB,");
  lines.push("            red: Double((hex >> 16) & 0xFF) / 255,");
  lines.push("            green: Double((hex >> 8) & 0xFF) / 255,");
  lines.push("            blue: Double(hex & 0xFF) / 255,");
  lines.push("            opacity: alpha");
  lines.push("        )");
  lines.push("    }");
  lines.push("}");

  return lines.join("\n");
}

function generateKotlin(tokens: TokenMap): string {
  const lines: string[] = [
    "// Auto-generated from design-tokens/ — do not edit manually",
    "package ai.seaclaw.app.ui",
    "",
    "import androidx.compose.ui.graphics.Color",
    "import androidx.compose.ui.unit.dp",
    "import androidx.compose.ui.unit.sp",
    "",
    "object SCTokens {",
  ];

  // Dark
  lines.push("    // Colors (Dark)");
  lines.push("    object Dark {");
  const darkKeys = Object.keys(tokens).filter(
    (k) =>
      k.startsWith("dark.") &&
      !k.includes("shadow") &&
      typeof tokens[k] === "string" &&
      ((tokens[k] as string).startsWith("#") ||
        (tokens[k] as string).startsWith("rgba")),
  );
  for (const k of darkKeys.sort()) {
    const v = tokens[k] as string;
    const name = toKotlinCase(k.replace("dark.", ""));
    const color = v.startsWith("#") ? hexToKotlin(v) : colorToKotlin(v);
    lines.push(`        val ${name} = Color(${color})`);
  }
  lines.push("    }");
  lines.push("");

  // Light
  lines.push("    // Colors (Light)");
  lines.push("    object Light {");
  const lightKeys = Object.keys(tokens).filter(
    (k) =>
      k.startsWith("light.") &&
      !k.includes("shadow") &&
      typeof tokens[k] === "string" &&
      ((tokens[k] as string).startsWith("#") ||
        (tokens[k] as string).startsWith("rgba")),
  );
  for (const k of lightKeys.sort()) {
    const v = tokens[k] as string;
    const name = toKotlinCase(k.replace("light.", ""));
    const color = v.startsWith("#") ? hexToKotlin(v) : colorToKotlin(v);
    lines.push(`        val ${name} = Color(${color})`);
  }
  lines.push("    }");
  lines.push("");

  // Spacing
  lines.push("    // Spacing");
  const spaceMap: Record<string, string> = {
    "spacing.xs": "spaceXs",
    "spacing.sm": "spaceSm",
    "spacing.md": "spaceMd",
    "spacing.lg": "spaceLg",
    "spacing.xl": "spaceXl",
    "spacing.2xl": "space2xl",
  };
  for (const [path, name] of Object.entries(spaceMap)) {
    const v = tokens[path] as string | undefined;
    if (v) {
      const px = dimToPx(v);
      if (px != null) lines.push(`    val ${name} = ${px}.dp`);
    }
  }
  lines.push("");

  // Radius
  lines.push("    // Radius");
  const radiusMap: Record<string, string> = {
    "radius.sm": "radiusSm",
    "radius.md": "radiusMd",
    "radius.lg": "radiusLg",
    "radius.xl": "radiusXl",
  };
  for (const [path, name] of Object.entries(radiusMap)) {
    const v = tokens[path] as string | undefined;
    if (v) {
      const px = dimToPx(v);
      if (px != null) lines.push(`    val ${name} = ${px}.dp`);
    }
  }
  lines.push("");

  // Font sizes
  lines.push("    // Font sizes");
  const sizeMap: Record<string, string> = {
    "fontSize.xs": "textXs",
    "fontSize.sm": "textSm",
    "fontSize.base": "textBase",
    "fontSize.lg": "textLg",
    "fontSize.xl": "textXl",
  };
  for (const [path, name] of Object.entries(sizeMap)) {
    const v = tokens[path] as string | undefined;
    if (v) {
      const px = dimToPx(v);
      if (px != null) lines.push(`    val ${name} = ${px}.sp`);
    }
  }
  lines.push("}");
  return lines.join("\n");
}

function generateCHeader(_tokens: TokenMap): string {
  return `/* Auto-generated from design-tokens/ — do not edit manually */
#ifndef SC_DESIGN_TOKENS_H
#define SC_DESIGN_TOKENS_H

/* ANSI 256-color codes for semantic colors */
#define SC_COLOR_ACCENT   "\\033[38;5;209m"
#define SC_COLOR_SUCCESS  "\\033[38;5;78m"
#define SC_COLOR_WARNING  "\\033[38;5;178m"
#define SC_COLOR_ERROR    "\\033[38;5;196m"
#define SC_COLOR_INFO     "\\033[38;5;69m"
#define SC_COLOR_MUTED    "\\033[38;5;245m"
#define SC_COLOR_FAINT    "\\033[38;5;240m"
#define SC_COLOR_RESET    "\\033[0m"
#define SC_COLOR_BOLD     "\\033[1m"
#define SC_COLOR_DIM      "\\033[2m"

/* Box-drawing characters (UTF-8) */
#define SC_BOX_VERT       "\\xe2\\x94\\x82"  /* │ */
#define SC_BOX_HORIZ      "\\xe2\\x94\\x80"  /* ─ */
#define SC_BOX_TL         "\\xe2\\x94\\x8c"  /* ┌ */
#define SC_BOX_TR         "\\xe2\\x94\\x90"  /* ┐ */
#define SC_BOX_BL         "\\xe2\\x94\\x94"  /* └ */
#define SC_BOX_BR         "\\xe2\\x94\\x98"  /* ┘ */
#define SC_CHEVRON        "\\xe2\\x9d\\xaf"  /* ❯ */
#define SC_CHECK          "\\xe2\\x9c\\x93"  /* ✓ */
#define SC_CROSS          "\\xe2\\x9c\\x97"  /* ✗ */

#endif /* SC_DESIGN_TOKENS_H */
`;
}

function formatSwiftColor(val: string): string {
  if (val.startsWith("#")) {
    return `Color(hex: ${hexToSwift(val)})`;
  }
  const m = val.match(/rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*([\d.]+))?\)/);
  if (m) {
    const r = Math.round((parseInt(m[1], 10) / 255) * 10000) / 10000;
    const g = Math.round((parseInt(m[2], 10) / 255) * 10000) / 10000;
    const b = Math.round((parseInt(m[3], 10) / 255) * 10000) / 10000;
    const a = m[4] ? Math.round(parseFloat(m[4]) * 10000) / 10000 : 1;
    return `Color(red: ${r}, green: ${g}, blue: ${b}, opacity: ${a})`;
  }
  return "Color(hex: 0x000000)";
}

function toSwiftCase(s: string): string {
  const parts = s.split("-");
  return (
    parts[0].toLowerCase() +
    parts
      .slice(1)
      .map((p) => p.charAt(0).toUpperCase() + p.slice(1))
      .join("")
  );
}

function toKotlinCase(s: string): string {
  const parts = s.split("-");
  return (
    parts[0] +
    parts
      .slice(1)
      .map((p) => p.charAt(0).toUpperCase() + p.slice(1))
      .join("")
  );
}

main();
