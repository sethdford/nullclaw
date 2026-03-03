# ADR-0003: Phosphor Icons

- **Status**: accepted
- **Date**: 2025-12-01
- **Deciders**: design system team
- **Tags**: design-system, icons, phosphor

## Context

The UI used emoji characters as icons, which rendered inconsistently across platforms and lacked semantic meaning for screen readers.

## Decision

Adopt Phosphor Regular as the canonical icon library. Web dashboard uses SVG paths in `ui/src/icons.ts`. Native apps use SF Symbols (Apple) or Material Icons (Android) for platform-native feel.

## Consequences

### Positive

- Consistent rendering across platforms
- Scalable SVG format
- Accessible via ARIA attributes
- Single source of truth in icons.ts
- 1000+ icons available

### Negative

- Slightly more code than emoji
- Requires icon module import

### Neutral

- Native apps still use platform icons for best-in-class feel

## Alternatives Considered

### Lucide

- **Pros**: Clean, MIT license, Tree-shakeable
- **Cons**: Smaller icon library
- **Why rejected**: Fewer icons than Phosphor; less comprehensive

### Material Symbols

- **Pros**: Extensive library, good Android integration
- **Cons**: Google-centric design language
- **Why rejected**: Doesn't fit cross-platform neutrality; Google CDN dependency

### Custom SVGs

- **Pros**: Full control over design
- **Cons**: Maintenance burden for 100+ icons
- **Why rejected**: High maintenance; Phosphor covers needs

## References

- AGENTS.md §12.2 Icons
- ui/src/icons.ts
