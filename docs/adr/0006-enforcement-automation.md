---
title: ADR-0006: Enforcement Automation
---
# ADR-0006: Enforcement Automation

- **Status**: accepted
- **Date**: 2025-12-01
- **Deciders**: design system team, platform team
- **Tags**: design-system, automation, linting, ci

## Context

Design standards are only as good as their enforcement. Manual code review is insufficient to catch all violations across a growing codebase with multiple platforms.

## Decision

Implement a multi-layer enforcement stack: (1) Cursor rules for AI agents, (2) AGENTS.md contract, (3) Stylelint + ESLint rules for code, (4) CI gates for token drift and bundle size, (5) Pre-push hooks for local validation.

## Consequences

### Positive

- Near-zero chance of design drift
- Automated enforcement at every stage (write → commit → push → merge)
- Self-documenting standards

### Negative

- Initial setup complexity
- Requires maintaining multiple config files

### Neutral

- Human override possible via --no-verify for emergencies

## Alternatives Considered

### Manual code review only

- **Pros**: No tooling overhead
- **Cons**: Doesn't scale; violations slip through
- **Why rejected**: Insufficient for multi-platform consistency

### Design system package

- **Pros**: Encapsulated components, versioned
- **Cons**: Premature for current scale, adds distribution complexity
- **Why rejected**: Overkill; monorepo sufficient for now

### Storybook Chromatic

- **Pros**: Visual regression testing, component catalog
- **Cons**: Adds SaaS dependency, cost at scale
- **Why rejected**: Adds external dependency; prefer self-hosted CI

## References

- AGENTS.md §6 Agent Workflow
- AGENTS.md §8 Validation Matrix
- .githooks/ pre-commit and pre-push
- ui/scripts/check-bundle-size.sh
- design-tokens/check-drift.sh
