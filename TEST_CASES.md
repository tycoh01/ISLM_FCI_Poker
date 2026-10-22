# Conservative integration checks

1. Build `PokerAI-ISLM.sln` as Debug x64 in Visual Studio.
2. Run with the bundled CSV. Confirm FCI prints its PAG and the sample size is 202.
3. Confirm GDPGrowth--GovtGrowth and GDPGrowth--RealInterestRate have no retained PAG adjacency; the conservative rule must not assign their OLS coefficients as policy effects.
4. Confirm the recommendation preserves automatic stabilizers, avoids a numerical GDP multiplier for new discretionary moves, and asks for newer evidence before a pilot.
5. Confirm China, European Union, and Japan profiles display year-specific observed data and country-specific conservative plays.
6. Run with a missing CSV path; confirm a clear FCI error and nonzero exit.
7. Run with invalid alpha such as 1.2; confirm clear FCI error and nonzero exit.
