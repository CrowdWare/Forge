# Range

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[Range](Range.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [EditorSpinSlider](EditorSpinSlider.md)
- [ProgressBar](ProgressBar.md)
- [ScrollBar](ScrollBar.md)
- [Slider](Slider.md)
- [SpinBox](SpinBox.md)
- [TextureProgressBar](TextureProgressBar.md)

## Properties

This page lists **only properties declared by `Range`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| allow_greater | allowGreater | bool | - |
| allow_lesser | allowLesser | bool | - |
| exp_edit | expEdit | bool | - |
| max_value | maxValue | float | - |
| min_value | minValue | float | - |
| page | page | float | - |
| rounded | rounded | bool | - |
| step | step | float | - |
| value | value | float | - |

## Events

This page lists **only signals declared by `Range`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| changed | `on <id>.changed() { ... }` | - |
| value_changed | `on <id>.valueChanged(value) { ... }` | float value |

## Runtime Actions

This page lists **callable methods declared by `Range`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_as_ratio | `<id>.getAsRatio()` | - | float |
| get_max | `<id>.getMax()` | - | float |
| get_min | `<id>.getMin()` | - | float |
| is_greater_allowed | `<id>.isGreaterAllowed()` | - | bool |
| is_lesser_allowed | `<id>.isLesserAllowed()` | - | bool |
| is_ratio_exp | `<id>.isRatioExp()` | - | bool |
| is_using_rounded_values | `<id>.isUsingRoundedValues()` | - | bool |
| set_as_ratio | `<id>.setAsRatio(value)` | float value | void |
| set_exp_ratio | `<id>.setExpRatio(enabled)` | bool enabled | void |
| set_max | `<id>.setMax(maximum)` | float maximum | void |
| set_min | `<id>.setMin(minimum)` | float minimum | void |
| set_use_rounded_values | `<id>.setUseRoundedValues(enabled)` | bool enabled | void |
| set_value_no_signal | `<id>.setValueNoSignal(value)` | float value | void |
| unshare | `<id>.unshare()` | - | void |

## Attached Properties

These properties are declared by a parent provider and set on this element using the qualified syntax `<providerId>.property: value` or `ProviderType.property: value`.

### Provided by `TabContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent TabContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `TabContainer.title: "Caption"`. |

### Provided by `DockingContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent DockingContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `DockingContainer.title: "Caption"`. |

