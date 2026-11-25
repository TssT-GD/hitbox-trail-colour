# Hitbox Trail Input Colors v0.3.0

GD 2.2081 / Geode 5.11.2 / Windows x64. Independent overlay, not Mega Hack's internal trail.

Records player 1 samples while alive, but renders only after death; freezes until reset. The death hook saves the last collision position and postUpdate provides a fallback. Extra press/release samples remain enabled by default, including inputs routed through PlayerObject by CBF/CBS. Actual compatibility with CBF/CBS is untested.

Colors: red idle, green held, yellow press, blue release. Configure trail length and extra samples in Geode settings. Disable Mega Hack's own Hitbox Trail if unwanted.

Build: place project files in repository root, then run Build Windows in GitHub Actions. Download HitboxTrailInputColors artifact, extract the .geode file into geode/mods. Source only; not compiled or runtime tested here.
