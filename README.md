# Hitbox Trail Input Colors v0.2.0
GD 2.2081 / Geode 5.11.2 / Windows x64. Experimental, not compiled or tested in GD.

Independent overlay: disable Mega Hack Hitbox Trail to avoid duplicate trails.
Normal samples are still recorded in PlayLayer::postUpdate. Extra samples are captured immediately before each jump button state transition in PlayerObject::pushButton/releaseButton, rather than using a single flag per frame. Every transition is stored separately, even multiple transitions between normal samples. Red = idle, green = held, yellow = press, blue = release. Input markers are rendered above normal samples. Extra samples are enabled by default and count toward the trail length.

This does not identify CBF/CBS specifically: it records normal input transitions too. It is intended to support CBF/CBS implementations that invoke these PlayerObject functions at their actual input step. If a particular implementation bypasses these hooks or changes hook ordering, exact compatibility is not guaranteed. No interpolation or alteration of game physics. Regular samples are not guaranteed to represent individual physics ticks. Player 1 only; practice checkpoints and exact collision geometry not validated.

Upload all files including .github to GitHub and run Build Windows in Actions; install resulting .geode in Geometry Dash/geode/mods. Or use matching Geode SDK and geode build.

Test: compare ordinary clicks, CBS, and CBF separately, including a press and release between two normal samples; check reset and disabled state. 
