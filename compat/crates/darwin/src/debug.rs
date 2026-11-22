//! Debug channels, named the way `WINEDEBUG` names them.
//!
//! The spelling and the parser live in `prefix`, which sits below every
//! crate that needs a channel.

/// Whether `channel` is switched on.
pub fn enabled(channel: &str) -> bool {
    prefix::debug::enabled(channel)
}
