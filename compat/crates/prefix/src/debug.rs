//! Debug channels, named the way `WINEDEBUG` names them.
//!
//! `ROSEMARY_DEBUG` holds a comma-separated list of `+channel` or `-channel`,
//! read left to right so a later entry overrides an earlier one. `all` stands
//! for every channel, and a bare name means `+name`. Nothing is on by default.

use std::sync::OnceLock;

fn spec() -> &'static [(bool, String)] {
    static SPEC: OnceLock<Vec<(bool, String)>> = OnceLock::new();
    SPEC.get_or_init(|| {
        let Some(value) = std::env::var_os("ROSEMARY_DEBUG") else {
            return Vec::new();
        };
        value
            .to_string_lossy()
            .split(',')
            .filter_map(|item| {
                let item = item.trim();
                let (on, name) = match item.as_bytes().first() {
                    Some(b'-') => (false, &item[1..]),
                    Some(b'+') => (true, &item[1..]),
                    _ => (true, item),
                };
                let name = name.trim().to_ascii_lowercase();
                (!name.is_empty()).then_some((on, name))
            })
            .collect()
    })
}

/// Whether `channel` is switched on.
pub fn enabled(channel: &str) -> bool {
    let mut on = false;
    for (wanted, name) in spec() {
        if name == "all" || name == channel {
            on = *wanted;
        }
    }
    on
}
