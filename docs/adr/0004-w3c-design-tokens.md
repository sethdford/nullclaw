---
title: ADR-0004: W3C Design Tokens
---
# ADR-0004: W3C Design Tokens

- **Status**: accepted
- **Date**: 2025-12-01
- **Deciders**: design system team
- **Tags**: design-system, tokens, w3c, automation

## Context

Design values (colors, spacing, typography, motion) were scattered across CSS, Swift, Kotlin, and C files with no single source of truth, leading to drift.

## Decision

Adopt W3C Design Tokens Community Group format for all design tokens. A build pipeline (`design-tokens/build.ts`) compiles tokens to CSS, Swift, Kotlin, and C outputs.

## Consequences

### Positive

- Single source of truth for design values
- Automated platform parity
- Drift detection via CI
- Machine-readable format

### Negative

- Requires build step for token changes
- Adds a dependency (tsx)

### Neutral

- W3C format is an emerging standard, may evolve

## Alternatives Considered

### Style Dictionary

- **Pros**: Mature, widely adopted, many output formats
- **Cons**: Heavier dependency, different schema
- **Why rejected**: Heavier than needed; W3C format preferred for standards alignment

### Manual sync

- **Pros**: No build dependency
- **Cons**: Drift-prone, error-prone
- **Why rejected**: Doesn't scale; drift inevitable

### CSS-only tokens

- **Pros**: Simple, no build for web
- **Cons**: Doesn't cover native platforms (Swift, Kotlin, C)
- **Why rejected**: Need cross-platform token distribution

## References

- AGENTS.md §12.3 Design Tokens
- design-tokens/ directory
- design-tokens/check-drift.sh
