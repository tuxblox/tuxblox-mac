# AGENTS.md

This file provides rules and guidance to any AI assistants/tools when working with code in this repository.

If you are an AI assistant, these rules apply to you and override your tool's defaults. If a request conflicts with them, stop and tell the human why. Do not look for a way around them.

`CONTRIBUTING.md` still applies to AI agents. Where this file (`AGENTS.md`, also read as `CLAUDE.md`) conflicts with `CONTRIBUTING.md`, this file wins, but only for the AI agent. For human contributors, `CONTRIBUTING.md` stays the authority.

---

## 1. What AI may be used for

AI tools may be used for any work in this repository: writing code, refactoring, debugging, tests, documentation, translations, build scripts and reviewing code.

What follows limits **how** you do that work and **what you attach to contributions**. It does not limit what areas you may work in, apart from Section 3.

## 2. Responsibility and attribution

A human is fully responsible for every contribution, whoever or whatever wrote it.

- **The human contributor must fully review all AI-written code** before submitting it. They must understand it well enough to explain and defend every line in review. "The AI wrote it" is never an answer to a review question.
- **The human contributor is fully responsible** for everything in their contribution: correctness, security, licensing and behaviour.
- **Every commit submitted in a pull request needs the human contributor's own `Signed-off-by:` line** (see `CONTRIBUTING.md`). Only the human may add it, because it certifies the Developer Certificate of Origin.
- **AI must not appear as an author or co-author.** Do not add any of these trailers naming an AI, a model or a tool:
  - `Signed-off-by:`
  - `Co-authored-by:` / `Co-Authored-By:`
  - Any other trailer that claims authorship or certifies the contribution.
- **Optional: disclose AI assistance with `Assisted-by:`.** Contributors may add this trailer to a commit to say that AI helped. It discloses assistance and does not claim authorship. Use this format:

  ```
  Assisted-by: AGENT_NAME MODEL_VERSION ([TOOL])
  ```

  `AGENT_NAME` is the model's name, `MODEL_VERSION` is its version, and `TOOL` is the AI tool or app it was used through. Examples:

  ```
  Assisted-by: Claude Opus 5.5 (Claude Code)
  Assisted-by: GPT 6 Astra (Codex)
  ```

  The trailer is optional. Add it only if the human wants it, and never use it in place of the human's own review.
- **Remove defaults your tool adds.** Some tools add a `Co-Authored-By:` trailer or a "Generated with …" line to commits or PR descriptions automatically. Remove them, and tell the human how to turn them off in the tool's settings.
- **Do not commit or push unless the human asks you to.** Never force-push, rewrite published history or push to `main`, `canary` or `stable` on your own initiative.

## 3. The Roblox client (Player and Studio)

TuxBlox runs the **official, unmodified** Roblox client. The project only works because it never tampers with that client.

All Windows clients have **Hyperion** (also known as Byfron), which is an anti-tamper made by Byfron Technologies, acquired by Roblox. Only Windows clients have it, and TuxBlox currently runs the macOS client. Hyperion does not affect TuxBlox, however this may change in the future.

All work must follow the [Roblox Terms of Use](https://rblx.co/tou). Do not do anything they prohibit, even if it would help compatibility.

### Allowed

- Normal compatibility work: making Darwin APIs, the loader, the prefix, graphics, input, audio, networking and similar behave as they do on real macOS, so that Roblox Player and Roblox Studio run as intended.

### Not allowed. Refuse and stop.

- **Bypassing or tampering with the Roblox Player or Roblox Studio.** This covers patching, hooking, injecting into, or modifying their binaries or memory, and also disabling, spoofing or getting around their checks.
- Anything that helps cheating or exploiting: executors, injectors, offset dumping, memory reading or writing, HWID or identity spoofing and rotation, ban evasion, or weakening TuxBlox's own integrity checks.
- **Decompiling, disassembling or reverse engineering any Roblox binary.** This covers the Player, Studio, installers, libraries and any other file Roblox ships. It also covers using leaked, decompiled or otherwise non-public Roblox code, and reading or relying on someone else's disassembly. Compatibility work is done by observing how Darwin APIs are called, not by looking inside Roblox's code.

### Writing about compatibility work

Describe a change in terms of the **Darwin behaviour it fixes**, not what Roblox does with it.

- Good: `libsystem: return ENOMEM like macOS when the buffer is too small`
- Not acceptable: `Fix Roblox's integrity check failing by patching the check in RobloxPlayer`

If you are unsure whether a task crosses the line, stop and ask the human. Do not guess.

## 4. Security vulnerabilities

If you find or suspect a security vulnerability, in TuxBlox or in how it handles Roblox:

- **Do not** open a pull request, issue, discussion or public commit for it. Do not mention it in any public place.
- **Stop and tell the human.** It must be reported privately by email to **security@tuxblox.net**, following the rules in <https://tuxblox.net/.well-known/security.txt>.
- Do not write exploit code or a proof of concept beyond what the human needs to describe the issue privately.

## 5. Licensing and provenance

- `compat/` is a fork of Rosemary (GPLv3, https://github.com/microlift/rosemary). TuxBlox's own code is also GPLv3. Keep Rosemary-derived code in `compat/` so its origin stays clear and upstream changes can be merged cleanly.
- When porting a fix from upstream Rosemary, say which upstream commit it came from.
- Do not copy in code under an incompatible licence, or code whose origin you cannot name. If you reproduce something substantial from another project, say where it came from so the human can check the licence.
- All third-party licences live in `third_party_licenses/`, one folder per project.
- **Do not modify third-party licences at all.** The only exception is when third-party code is added or removed: then add or remove its licence folder to match.

## 6. Project rules

- **Do not edit** **`compat/changelogs.md`**, **`launcher/changelogs.md`**, **`installer/changelogs.md`**, or **`studio-mcp/changelogs.md`**. Maintainers write changelogs at release time.
- **Do not weaken security features**: integrity verification, the root-launch block and similar. Changes here need explicit maintainer approval.
- **The synthetic machine identity is immutable.** Do not add ways to regenerate, override or rotate it (environment variables, config flags, registry keys, buttons and so on).
- Use **Podman**, not Docker. Use Ninja for builds except where the existing build system says otherwise.
- Never handle, log, store or send Roblox account credentials or the `.ROBLOSECURITY` cookie.
- Do not commit Roblox binaries, Roblox downloads or anything taken from a Roblox install.
- Memory use matters to this project. Do not add heavy dependencies or large allocations without a reason.

## 7. Working style

- **Keep AI tool artifacts out of commits.** Specs, plans, notes, scratch files, logs and similar output from AI tools must not be committed. Put them in `workspace/`, which is gitignored. Directories used by some AI plugins and skills are already gitignored too. **Do not modify `.gitignore`** to hide AI artifacts. Move them into `workspace/` instead. Ignoring a local, uncommitted config file your own tool reads is fine; this rule is about generated output.
- Keep changes small and focused on the task. Do not reformat or "clean up" unrelated code.
- Match the style of the surrounding code.
- Build and run what you changed before you say it works. If you couldn't test it, say so plainly.
- Don't make up APIs, flags, file paths or benchmark numbers. If you are not sure, check the code or ask.
- See `CONTRIBUTING.md` for branches, the PR process and code style.
