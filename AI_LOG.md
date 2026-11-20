# AI Log

A running record of the prompts I gave AI tools, what they produced, and what I kept,
changed, or rejected. Goal: 10+ entries, including at least 3 where the AI was wrong or
incomplete and I caught it.

Template for each entry:

```
## Entry N - YYYY-MM-DD - Tool
**Prompt (exact):**
**What I used it for:**
**What the AI produced:**
**What I kept / changed / rejected, and why:**
**Did the AI get something wrong?** (yes/no - what and how I caught it)
```

---

## Entry 1 - 2026-11-16 - Claude Code

**Prompt (exact):** _(paste the message you sent Claude here)_

**What I used it for:** Getting my tools and project set up. I had never used GitHub or
built a GUI program before. My professor suggested adding a graphical interface to my
Recipe Keeper proposal.

**What the AI produced:**
- Checked my computer: Git and Visual Studio (with C++ tools) were already installed.
- Recommended Dear ImGui as the GUI library and created a starter project
  (`CMakeLists.txt`, `src/main.cpp`) that opens a window with a text box and a button.
- Created `README.md`, `.gitignore`, and this AI log, and set up a Git repository.

**What I kept / changed / rejected, and why:** _(fill in)_

**Did the AI get something wrong?** The first build failed with
`error C1083: Cannot open source file: 'CMakeCXXCompilerId.cpp'`. The cause was the
project sitting in a folder with a very long path (Windows has a ~260 character path
limit). Fix: keep the project in a short path like `C:\dev\recipe-keeper`.
