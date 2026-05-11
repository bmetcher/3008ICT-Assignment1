fof(p1, conjecture, ((p => q) <=> (~q => ~p))).
fof(p2, conjecture, (~~p <=> p)).
fof(p3, conjecture, (~(p => q) => (q => p))).
fof(p4, conjecture, ((~p => q) <=> (~q => p))).
fof(p5, conjecture, (((p | q) => (p | r)) => (p | (q => r)))).
fof(p6, conjecture, (p | ~p)).
fof(p7, conjecture, (p | ~~~p)).
fof(p8, conjecture, (((p => q) => p) => p)).
fof(p9, conjecture, ((((p | q) & (~p | q)) & (p | ~q)) => ~(~p | ~q))).
fof(p10, conjecture, (((q => r) & ((r => (p & q)) & (p => (q | r)))) => (p <=> q))).
fof(p11, conjecture, (p <=> p)).
fof(p12, conjecture, (((p <=> q) <=> r) <=> (p <=> (q <=> r)))).
fof(p13, conjecture, ((p | (q & r)) <=> ((p | q) & (p | r)))).
fof(p14, conjecture, ((p <=> q) <=> ((q | ~p) & (~q | p)))).
fof(p15, conjecture, ((p => q) <=> (~p | q))).
fof(p16, conjecture, ((p => q) | (q => p))).
fof(p17, conjecture, (((p & (q => r)) => s) <=> (((~p | q) | s) & ((~p | ~r) | s)))).
fof(p18, conjecture, (?[Y]: ![X]: (f(Y) => f(X)))).
fof(p19, conjecture, (?[X]: ![Y,Z]: ((p(Y) => q(Z)) => (p(X) => q(X))))).
fof(p20, conjecture, (((![X,Y]: ?[Z]: ![W]: ((p(X) & q(Y)) => (r(Z) & s(W)))) & (?[X,Y]: (p(X) & q(Y)))) => ?[Z]: r(Z))).
fof(p21, conjecture, (((?[X]: (p <=> f(X))) & (?[X]: (q <=> f(X)))) => ((p <=> q) => ?[X]: (q <=> f(X))))).
fof(p22, conjecture, ((![X]: (p <=> f(X))) => (p <=> ![X]: f(X)))).
fof(p23, conjecture, ((![X]: (p | f(X))) <=> (p | ![X]: f(X)))).
fof(p24, conjecture, ((((~?[X]: (s(X) & q(X))) & (![X]: (p(X) => (q(X) | r(X))))) & ((~?[X]: p(X)) => ?[X]: q(X))) & (![X]: ((q(X) | r(X)) => s(X))) => ?[X]: (p(X) & r(X)))).
fof(p25, conjecture, (((?[X]: p(X)) & (![X]: (f(X) => (~g(X) & r(X))))) & ((![X]: (p(X) => (g(X) & f(X)))) & ((![X]: (p(X) => h(X))) | ?[X]: (p(X) & h(X)))) => ?[X]: (h(X) & p(X)))).
fof(p26, conjecture, ((((?[X]: p(X)) <=> (?[X]: q(X))) & ![X,Y]: ((p(X) & q(Y)) => (r(X) <=> s(Y)))) => (![X]: (p(X) => r(X)) <=> ![X]: (q(X) => s(X))))).
fof(p27, conjecture, (((?[X]: (f(X) & ~g(X))) & (![X]: (f(X) => h(X))) & (![X]: ((j(X) & i(X)) => f(X))) & ((?[X]: (h(X) & ~g(X))) => ![X]: (i(X) => ~h(X)))) => ![X]: (j(X) => ~i(X)))).
fof(p28, conjecture, (((![X]: (p(X) => ![X]: q(X))) & ((![X]: (q(X) | r(X))) => ?[X]: (q(X) & s(X))) & ((?[X]: s(X)) => ![X]: (f(X) => g(X)))) => ![X]: ((p(X) & f(X)) => g(X)))).
fof(p29, conjecture, (((?[X]: f(X)) & (?[X]: g(X))) => ((![X]: (f(X) => h(X)) & ![X]: (g(X) => j(X))) <=> ![X,Y]: ((f(X) & g(Y)) => (h(X) & j(Y)))))).
fof(p30, conjecture, (((![X]: ((f(X) | g(X)) => ~h(X))) & (![X]: ((g(X) => ~i(X)) => (f(X) & h(X))))) => ![X]: i(X))).
fof(p31, conjecture, (((~?[X]: (f(X) & (g(X) | h(X)))) & (?[X]: (i(X) & f(X))) & (![X]: (~h(X) => j(X)))) => ?[X]: (i(X) & j(X)))).
fof(p32, conjecture, (((![X]: ((f(X) & (g(X) | h(X))) => i(X))) & (![X]: ((i(X) & h(X)) => j(X))) & (![X]: (k(X) => h(X)))) => ![X]: ((f(X) & k(X)) => j(X)))).
fof(p33, conjecture, ((![X]: ((p(a) & (p(X) => p(b))) => p(c))) <=> (![X]: ((~p(a) | (p(X) | p(c))) & (~p(a) | (~p(b) | p(c))))))).
fof(p34, conjecture, ((((?[X]: ![Y]: (p(X) <=> p(Y))) <=> ((?[X]: q(X)) <=> ![Y]: q(Y))) <=> ((?[X]: ![Y]: (q(X) <=> q(Y))) <=> ((?[X]: p(X)) <=> ![Y]: p(Y)))))).
fof(p35, conjecture, (?[X,Y]: (p(X,Y) => ![U,V]: p(U,V)))).
fof(p36, conjecture, (((![X]: ?[Y]: f(X,Y)) & (![X]: ?[Y]: g(X,Y)) & (![X,Y]: ((f(X,Y) | g(X,Y)) => (![Z]: ((f(Y,Z) | g(Y,Z)) => h(X,Z)))))) => ![X]: ?[Y]: h(X,Y))).
fof(p37, conjecture, (((![Z]: ?[W]: ![X]: ?[Y]: ((p(X,Z) & p(Y,W)) & (p(Y,Z) & (p(X,Y) => q(X,Y))))) & (![X,Z]: (~p(X,Z) => ?[Y]: q(Y,Z))) & (((?[X,Y]: q(X,Y)) => ![X]: r(X,X)))) => ![X]: ?[Y]: r(X,Y))).
fof(p38, conjecture, ((![X]: ((p(a) & (p(X) => ?[Y]: (p(Y) & r(X,Y)))) => ?[Z,W]: (p(Z) & (r(X,W) & r(W,Z))))) <=> (![X]: ((~p(a) | p(X) | ?[Z,W]: (p(Z) & (r(X,W) & r(W,Z)))) & (~p(a) | (~?[Y]: (p(Y) & r(X,Y))) | ?[Z,W]: (p(Z) & (r(X,W) & r(W,Z)))))))).
fof(p39, conjecture, (~?[X]: ![Y]: (f(Y,X) <=> ~f(Y,Y)))).
fof(p40, conjecture, (((?[Y]: ![X]: (f(X,Y) <=> f(X,X)))) => ~![X]: ?[Y]: ![Z]: (f(Z,Y) <=> ~f(Z,X)))).
fof(p41, conjecture, ((![Z]: ?[Y]: ![X]: (f(X,Y) <=> (f(X,Z) & ~f(X,X)))) => ~?[Z]: ![X]: f(X,Z))).
fof(p42, conjecture, (~?[Y]: ![X]: (f(X,Y) <=> ~?[Z]: (f(X,Z) & f(Z,X))))).
fof(p43, conjecture, ((![X,Y]: (q(X,Y) <=> ![Z]: (f(Z,X) <=> f(Z,Y)))) => ![X,Y]: (q(X,Y) <=> q(Y,X)))).
fof(p44, conjecture, (((![X]: (f(X) => ((?[Y]: (g(Y) & h(X,Y))) & (?[Y]: (g(Y) & ~i(X,Y)))))) & (?[X]: (j(X) & ![Y]: (g(Y) => h(X,Y))))) => ?[X]: (j(X) & ~f(X)))).
fof(p45, conjecture, (((![X]: ((f(X) & ![Y]: ((g(Y) & h(X,Y)) => j(X,Y))) => ![Y]: ((g(Y) & h(X,Y)) => k(Y)))) & (~?[Y]: (l(Y) & k(Y))) & (?[X]: (f(X) & ![Y]: (h(X,Y) => l(Y)) & ![Y]: ((g(Y) & h(X,Y)) => j(X,Y))))) => ?[X]: (f(X) & ~?[Y]: (g(Y) & h(X,Y))))).
fof(p46, conjecture, (((![X]: ((f(X) & ![Y]: ((f(Y) & h(Y,X)) => g(Y))) => g(X))) & ((?[X]: (f(X) & ~g(X))) => (?[X]: (f(X) & ~g(X) & ![Y]: ((f(Y) & ~g(Y)) => j(X,Y))))) & (![X,Y]: ((f(X) & f(Y) & h(X,Y)) => ~j(Y,X)))) => ![X]: (f(X) => g(X)))).
fof(p47, conjecture, ((![X]: ((p1(X) => p0(X)) & ?[Y]: q1(Y,X))) & (![X]: ((p2(X) => p0(X)) & ?[Y]: q2(Y,X))) & (![X]: (p1(X) <=> p2(X))) & (![X,Y]: (q1(X,Y) => q1(Y,X))) => ?[X]: (p0(X) & p1(X)))).