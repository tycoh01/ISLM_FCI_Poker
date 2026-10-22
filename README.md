# Conservative Poker AI + FCI + IS-LM (Visual Studio C++11-compatible source)

A single Visual Studio console project combines the existing FCI causal-discovery implementation and the opponent-aware Poker AI policy explorer. It applies a conservative decision rule in the IS-LM context using observed data.

## Open and run

1. Open `PokerAI-ISLM.sln` in Visual Studio 18 with the Desktop development with C++ workload.
2. Select `Debug | x64`, build, and run. The supplied `fci_real_input.csv` is copied to the output directory.
3. Optional command line: `PokerAI-ISLM.exe [observed_data.csv] [alpha=0.05] [max_conditioning=2]`.

The program runs FCI, prints the estimated PAG, and then applies the conservative policy rule to China, the European Union, and Japan. Policy action labels “fold / call / small pilot” are analogies for decision thresholds, not poker play in an actual card game.

## Conservative FCI gate

The rule checks whether the FCI PAG contains a directed policy-instrument-to-GDP-growth edge. Only such a directed edge permits considering a small, reversible pilot. An unresolved edge leads to monitoring only; no retained edge means the data/model does not support a quantitative causal claim. It is **not** interpreted as proof that the economic effect is zero.

With the supplied historical sample, the FCI graph retains associations of GDP growth with consumption and investment growth, but does not retain a GDP-growth edge to government-spending growth or the real interest rate. Accordingly, the project does not assign a GDP multiplier to a new discretionary IS-LM intervention. It favors keeping automatic stabilizers, monitoring observed activity, preserving flexibility, and collecting newer evidence before a policy pilot.

## Real data and model limits

`fci_real_input.csv` contains 202 quarterly rows derived from observed U.S. macrodata, 1959Q2–2009Q3. Real GDP, consumption, investment, government spending, and real M1 growth are transformed as 400 times the log change; the other variables are source observations. The source dataset covers 203 quarters from 1959Q1–2009Q3. Metadata identifies FRED as source for all series except unemployment, which is from BLS.

The printed historical IS reduced form is `GDP growth = intercept + 0.0541 × lagged real rate + 0.0464 × government-spending growth` (n=200, R²=0.013). It is a weak descriptive regression, not a structural IS-LM equation or causal estimate. FCI is an educational compact implementation: it approximates Possible-D-Sep and applies only orientation rules R1/R2. PAG circle endpoints mean orientation remains unresolved. Neither the historical sample nor this compact model provides a current 2027 forecast.

Opponent profiles use observed WTO Tariff & Trade Data / UN Comtrade exports: China (2024), EU (2025), Japan (2025). The years differ. Their U.S. destination shares are shares of each counterpart's export destinations, not measures of U.S. import dependence. The country-specific conservative responses are policy hypotheses, not estimated effects.

## Poker AI attribution

Poker AI was not invented by Stanford alone. This project uses poker-inspired conservative decision language and strategy framing; it does not reproduce a full poker solver or Stanford research implementation. Stanford has published work on adversarial strategy generation for poker agents, alongside the wider research history at institutions including the University of Alberta and Carnegie Mellon.

## Sources

- FRED/BLS macrodata metadata and data: https://github.com/statsmodels/statsmodels/blob/main/statsmodels/datasets/macrodata/data.py and https://github.com/statsmodels/statsmodels/blob/main/statsmodels/datasets/macrodata/macrodata.csv
- WTO / UN Comtrade profiles: https://ttd.wto.org/en/profiles/china ; https://ttd.wto.org/en/profiles/european-union ; https://ttd.wto.org/en/profiles/japan
- FCI reference: https://doi.org/10.7551/mitpress/2006.003.0009
- Stanford poker-agent project: https://cs224r.stanford.edu/projects/pdfs/Final_Report%20%281%29.pdf
- University of Alberta Computer Poker Research Group: https://poker.cs.ualberta.ca/
- Carnegie Mellon Libratus: https://www.cs.cmu.edu/news/2017/carnegie-mellon-artificial-intelligence-beats-top-poker-pros
