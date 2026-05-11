import random
import sys

ATOMS = ['p', 'q', 'r', 's', 't']
CONNECTIVES = ['&', '|', '=>', '<=>']

HARD_FOL_TEMPLATES = [
    # Nested ∀∃ alternation with binary predicate
    lambda p: f"((![X]: ?[Y]: {p}(X, Y)) => (![X]: ?[Y]: {p}(X, Y)))",
    
    # Triple alternation
    lambda p: f"(![X]: ?[Y]: ![Z]: ({p}(X, Y) | ~{p}(X, Y)))",
    
    # Quantifier swap that requires multiple instantiations
    lambda p, q: f"((![X]: ({p}(X) => {q}(X))) & (![X]: ({q}(X) => {p}(X))) => (![X]: ({p}(X) <=> {q}(X))))",
    
    # Nested quantifiers with implications
    lambda p, q: f"((![X]: ?[Y]: ({p}(X) => {q}(Y))) | (?[X]: ![Y]: ({p}(X) & ~{q}(Y))) | (![X]: ![Y]: ({p}(X) => {q}(Y))))",
    
    # Forces multiple uses of same forall
    lambda p, q: f"((![X]: ({p}(X) => {q}(X))) => ((![X]: {p}(X)) => ((?[X]: {p}(X)) => ?[X]: {q}(X))))",
    
    # Binary predicate with reflexivity-like structure
    lambda p: f"((![X,Y]: ({p}(X, Y) => {p}(Y, X))) => (![X]: ({p}(X, X) => {p}(X, X))))",
    
    # Drinker's paradox variant (genuinely tricky for naive solvers)
    lambda p: f"(?[X]: ({p}(X) => ![Y]: {p}(Y)))",
    
    # Nested quantifiers with multiple predicates
    lambda p, q, r: f"((![X]: ({p}(X) => {q}(X))) & (![X]: ({q}(X) => {r}(X))) => (![X]: ({p}(X) => {r}(X))))",
    
    # Forces gamma rule to fire many times
    lambda p, q: f"((![X,Y]: (({p}(X) & {p}(Y)) => {q}(X, Y))) & (?[X]: {p}(X)) => ?[X,Y]: {q}(X, Y))",
    
    # Deep alternation
    lambda p: f"((?[X]: ![Y]: {p}(X, Y)) => (![Y]: ?[X]: {p}(X, Y)))",
]

def generate_hard_fol_tautology():
    """Generate a deeply quantified FOL tautology designed to stress the baseline."""
    template = random.choice(HARD_FOL_TEMPLATES)
    pred_pool = ['p', 'q', 'r', 's', 't']
    
    # Try with 1, 2, then 3 predicates
    for arity in [1, 2, 3]:
        try:
            preds = random.sample(pred_pool, arity)
            return template(*preds)
        except TypeError:
            continue
    return None

def generate_hard_fol_dataset(n_target, seed=None):
    if seed is not None:
        random.seed(seed)
    
    results = []
    attempts = 0
    while len(results) < n_target:
        attempts += 1
        f = generate_hard_fol_tautology()
        if f is not None:
            results.append(f)
        if attempts > n_target * 100:
            break
    return results

def random_formula(depth, atoms):
    """Build a random formula of approximately the given depth."""
    if depth == 0 or (depth > 0 and random.random() < 0.3):
        return random.choice(atoms)
    
    choice = random.random()
    if choice < 0.15:
        return f"~{random_formula(depth - 1, atoms)}"
    else:
        op = random.choice(CONNECTIVES)
        left = random_formula(depth - 1, atoms)
        right = random_formula(depth - 1, atoms)
        return f"({left} {op} {right})"

def tokenise(s):
    tokens = []
    i = 0
    while i < len(s):
        c = s[i]
        if c.isspace():
            i += 1
        elif c == '(':
            tokens.append('('); i += 1
        elif c == ')':
            tokens.append(')'); i += 1
        elif c == '~':
            tokens.append('~'); i += 1
        elif c == '&':
            tokens.append('&'); i += 1
        elif c == '|':
            tokens.append('|'); i += 1
        elif s[i:i+3] == '<=>':
            tokens.append('<=>'); i += 3
        elif s[i:i+2] == '=>':
            tokens.append('=>'); i += 2
        elif c.isalpha():
            j = i
            while j < len(s) and s[j].isalnum():
                j += 1
            tokens.append(s[i:j])
            i = j
        else:
            i += 1
    return tokens

def parse_iff(tokens, pos, assignment):
    left, pos = parse_imp(tokens, pos, assignment)
    while pos < len(tokens) and tokens[pos] == '<=>':
        right, pos = parse_imp(tokens, pos + 1, assignment)
        left = (left == right)
    return left, pos

def parse_imp(tokens, pos, assignment):
    left, pos = parse_or(tokens, pos, assignment)
    if pos < len(tokens) and tokens[pos] == '=>':
        right, pos = parse_imp(tokens, pos + 1, assignment)
        left = (not left) or right
    return left, pos

def parse_or(tokens, pos, assignment):
    left, pos = parse_and(tokens, pos, assignment)
    while pos < len(tokens) and tokens[pos] == '|':
        right, pos = parse_and(tokens, pos + 1, assignment)
        left = left or right
    return left, pos

def parse_and(tokens, pos, assignment):
    left, pos = parse_not(tokens, pos, assignment)
    while pos < len(tokens) and tokens[pos] == '&':
        right, pos = parse_not(tokens, pos + 1, assignment)
        left = left and right
    return left, pos

def parse_not(tokens, pos, assignment):
    if pos < len(tokens) and tokens[pos] == '~':
        val, pos = parse_not(tokens, pos + 1, assignment)
        return (not val), pos
    return parse_atom(tokens, pos, assignment)

def parse_atom(tokens, pos, assignment):
    if tokens[pos] == '(':
        val, pos = parse_iff(tokens, pos + 1, assignment)
        assert tokens[pos] == ')'
        return val, pos + 1
    return assignment[tokens[pos]], pos + 1

def evaluate(formula, assignment):
    tokens = tokenise(formula)
    result, _ = parse_iff(tokens, 0, assignment)
    return result

def is_tautology(formula, atoms):
    n = len(atoms)
    for i in range(2 ** n):
        assignment = {}
        for j, atom in enumerate(atoms):
            assignment[atom] = bool((i >> j) & 1)
        try:
            if not evaluate(formula, assignment):
                return False
        except Exception:
            return False
    return True

def used_atoms(formula, all_atoms):
    return [a for a in all_atoms if a in formula]

def generate_dataset(n_target, depth_range=(2, 6), max_atoms=4, seed=None):
    if seed is not None:
        random.seed(seed)
    
    tautologies = []
    attempts = 0
    
    while len(tautologies) < n_target:
        attempts += 1
        depth = random.randint(*depth_range)
        n_atoms = random.randint(2, max_atoms)
        atoms = ATOMS[:n_atoms]
        formula = random_formula(depth, atoms)
        
        present = used_atoms(formula, atoms)
        if not present:
            continue
        
        if is_tautology(formula, present):
            tautologies.append((formula, depth, len(present)))
        
        if attempts > n_target * 1000:
            print(f"Warning: only found {len(tautologies)} after {attempts} attempts", file=sys.stderr)
            break
    
    return tautologies

def generate_non_tautologies(n_target, depth_range, max_atoms, seed):
    if seed is not None:
        random.seed(seed)
    
    results = []
    attempts = 0
    while len(results) < n_target:
        attempts += 1
        depth = random.randint(*depth_range)
        n_atoms = random.randint(2, max_atoms)
        atoms = ATOMS[:n_atoms]
        formula = random_formula(depth, atoms)
        present = used_atoms(formula, atoms)
        if not present:
            continue
        if not is_tautology(formula, present):
            results.append((formula, depth, len(present)))
        if attempts > n_target * 100:
            break
    return results

def lift_to_fol(prop_formula, atoms_used):
    """Take a propositional tautology and lift it to FOL by replacing
       each atom with a unary predicate over a fresh variable, then
       universally closing."""
    # Pick a fresh variable for the universal closure
    var = 'X'
    
    # Replace each atom 'p' with 'p(X)'
    # Need to be careful: replace longest atoms first to avoid 'p' eating 'p1', etc.
    sorted_atoms = sorted(atoms_used, key=len, reverse=True)
    result = prop_formula
    for atom in sorted_atoms:
        # Replace whole-word occurrences
        result = replace_atom(result, atom, f"{atom}({var})")
    
    return f"![{var}]: {result}"

def replace_atom(formula, atom, replacement):
    """Replace `atom` with `replacement` only when it appears as a whole token."""
    result = []
    i = 0
    while i < len(formula):
        # Check if atom starts at position i and is a whole word
        if formula[i:i+len(atom)] == atom:
            # Check boundaries
            before_ok = i == 0 or not formula[i-1].isalnum()
            after_ok = (i + len(atom) >= len(formula) or 
                       not formula[i + len(atom)].isalnum())
            if before_ok and after_ok:
                result.append(replacement)
                i += len(atom)
                continue
        result.append(formula[i])
        i += 1
    return ''.join(result)

# FOL templates that are known valid
FOL_TEMPLATES = [
    # Universal instantiation
    lambda p: f"((![X]: {p}(X)) => {p}(a))",
    # Existential generalization  
    lambda p: f"({p}(a) => (?[X]: {p}(X)))",
    # Reflexivity of implication
    lambda p: f"(![X]: ({p}(X) => {p}(X)))",
    # Self-iff
    lambda p: f"(![X]: ({p}(X) <=> {p}(X)))",
    # Excluded middle, lifted
    lambda p: f"(![X]: ({p}(X) | ~{p}(X)))",
    # De Morgan, lifted
    lambda p, q: f"(![X]: (~({p}(X) & {q}(X)) <=> (~{p}(X) | ~{q}(X))))",
    # Distribution
    lambda p, q: f"((![X]: ({p}(X) => {q}(X))) => ((![X]: {p}(X)) => (![X]: {q}(X))))",
    # Existential over disjunction
    lambda p, q: f"((?[X]: ({p}(X) | {q}(X))) <=> ((?[X]: {p}(X)) | (?[X]: {q}(X))))",
    # Universal over conjunction
    lambda p, q: f"((![X]: ({p}(X) & {q}(X))) <=> ((![X]: {p}(X)) & (![X]: {q}(X))))",
]

def generate_fol_tautology():
    """Generate a single FOL tautology using one of two methods."""
    if random.random() < 0.5:
        # Method 1: lift a propositional tautology
        depth = random.randint(2, 5)
        n_atoms = random.randint(2, 3)
        atoms = ATOMS[:n_atoms]
        # generate until we get a tautology
        for _ in range(100):
            f = random_formula(depth, atoms)
            present = used_atoms(f, atoms)
            if present and is_tautology(f, present):
                return lift_to_fol(f, present)
        return None
    else:
        # Method 2: instantiate a template with random predicates
        template = random.choice(FOL_TEMPLATES)
        # Pick predicate names
        pred_pool = ['p', 'q', 'r', 's', 't']
        # Determine arity by counting params (hack: try with 1, then 2)
        try:
            return template(random.choice(pred_pool))
        except TypeError:
            p, q = random.sample(pred_pool, 2)
            return template(p, q)

def generate_fol_dataset(n_target, seed=None):
    if seed is not None:
        random.seed(seed)
    
    results = []
    attempts = 0
    while len(results) < n_target:
        attempts += 1
        f = generate_fol_tautology()
        if f is not None:
            results.append(f)
        if attempts > n_target * 100:
            break
    return results

def write_tptp(tautologies, non_tautologies, filename):
    with open(filename, 'w') as f:
        for i, (formula, depth, n_atoms) in enumerate(tautologies, 1):
            f.write(f"fof(g{i}, conjecture, {formula}).\n")
        for i, (formula, depth, n_atoms) in enumerate(non_tautologies, 1):
            f.write(f"fof(g{i}, conjecture, {formula}).\n")

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else 'generated.p'
    
    # Propositional tiers
    print("Generating easy propositional batch...")
    easy = generate_dataset(200, depth_range=(2, 4), max_atoms=3, seed=42)
    
    print("Generating medium propositional batch...")
    medium = generate_dataset(300, depth_range=(4, 7), max_atoms=4, seed=43)
    
    print("Generating hard propositional batch...")
    hard = generate_dataset(400, depth_range=(6, 10), max_atoms=5, seed=44)
    
    # FOL tiers (using different seeds for variety)
    print("Generating easy FOL batch...")
    fol_easy = generate_fol_dataset(200, seed=51)
    
    print("Generating medium FOL batch...")
    fol_medium = generate_fol_dataset(300, seed=52)

    print("Generating hard FOL batch (designed to stress baseline)...")
    fol_hard = generate_hard_fol_dataset(400, seed=53)
    
    # Optional: non-tautologies for soundness testing
    print("Generating non-tautologies...")
    non_taut = generate_non_tautologies(100, depth_range=(3, 6), max_atoms=4, seed=99)
    
    # Write out with prefixes that indicate category
    with open(out, 'w') as f:
        for i, (formula, _, _) in enumerate(easy, 1):
            f.write(f"fof(pe{i}, conjecture, {formula}).\n")
        for i, (formula, _, _) in enumerate(medium, 1):
            f.write(f"fof(pm{i}, conjecture, {formula}).\n")
        for i, (formula, _, _) in enumerate(hard, 1):
            f.write(f"fof(ph{i}, conjecture, {formula}).\n")
        for i, formula in enumerate(fol_easy, 1):
            f.write(f"fof(fe{i}, conjecture, {formula}).\n")
        for i, formula in enumerate(fol_medium, 1):
            f.write(f"fof(fm{i}, conjecture, {formula}).\n")
        for i, formula in enumerate(fol_hard, 1):
            f.write(f"fof(fh{i}, conjecture, {formula}).\n")
        for i, (formula, _, _) in enumerate(non_taut, 1):
            f.write(f"fof(n{i}, conjecture, {formula}).\n")
    
    total = len(easy) + len(medium) + len(hard) + len(fol_easy) + len(fol_medium) + len(fol_hard) + len(non_taut)
    print(f"\nWrote to {out}:")
    print(f"  Propositional easy:    {len(easy)} (prefix pe)")
    print(f"  Propositional medium:  {len(medium)} (prefix pm)")
    print(f"  Propositional hard:    {len(hard)} (prefix ph)")
    print(f"  FOL easy:              {len(fol_easy)} (prefix fe)")
    print(f"  FOL medium:            {len(fol_medium)} (prefix fm)")
    print(f"  FOL hard:              {len(fol_hard)} (prefix fh)")
    print(f"  Non-tautologies:       {len(non_taut)} (prefix n)")
    print(f"  Total:                 {total}")