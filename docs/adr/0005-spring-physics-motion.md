---
title: ADR-0005: Spring Physics Motion
---
# ADR-0005: Spring Physics Motion

- **Status**: accepted
- **Date**: 2025-12-01
- **Deciders**: design system team
- **Tags**: design-system, animation, motion, physics

## Context

CSS animations using fixed durations and bezier curves felt mechanical and lacked the organic feel of native platform animations.

## Decision

Adopt spring physics-based motion as the primary animation model. Four spring presets (micro, standard, expressive, dramatic) are defined as tokens with stiffness/damping parameters. CSS uses `linear()` approximations; native platforms use their spring animation APIs.

## Consequences

### Positive

- Natural, organic feel matching Apple/Google quality
- Consistent motion language across platforms
- Gesture-driven animations possible

### Negative

- CSS spring approximation via `linear()` is not exact
- Requires modern browser support

### Neutral

- Falls back to standard easing on older browsers

## Alternatives Considered

### Fixed duration + cubic-bezier only

- **Pros**: Simple, widely supported, predictable
- **Cons**: Mechanical feel, lacks organic response
- **Why rejected**: Does not match quality bar of native platforms

### Lottie/Rive

- **Pros**: Rich designer-created animations
- **Cons**: Complexity, binary size, runtime overhead
- **Why rejected**: Overkill for UI transitions; size impact

### Web Animations API only

- **Pros**: Standard, flexible
- **Cons**: Doesn't cover native platforms (iOS, Android)
- **Why rejected**: Need cross-platform solution

## References

- AGENTS.md §12.4 Motion & Animation
- design-tokens/ motion and spring tokens
