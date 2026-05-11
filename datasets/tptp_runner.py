import os
import subprocess
import sys
import re

TPTP_DIR = sys.argv[1] if len(sys.argv) > 1 else "./filtering/candidates"
PARSER_BINARY = sys.argv[2] if len(sys.argv) > 2 else "./parser"
OUTPUT_FILE = sys.argv[3] if len(sys.argv) > 3 else "tptp_results.txt"
MAX_PROBLEMS = int(sys.argv[4]) if len(sys.argv) > 4 else 100
MAX_RATING = 0.50  # only keep problems easier than this

def is_candidate(filepath):
    name = os.path.basename(filepath)
    return name.endswith(".p") and "+" in name

def has_equality(content):
    lines = [l for l in content.split('\n') if not l.strip().startswith('%')]
    body = '\n'.join(lines)
    return ' = ' in body or ' != ' in body

def has_includes(content):
    return 'include(' in content

def get_rating(content):
    """Extract TPTP difficulty rating, or None if absent."""
    for line in content.split('\n'):
        if 'Rating' in line and ':' in line:
            try:
                rating_str = line.split(':', 1)[1].strip().split(' ')[0]
                return float(rating_str)
            except (ValueError, IndexError):
                continue
    return None

def run_one(filepath):
    """Copy the TPTP file to test.p, run parser, return (status, time_ms)."""
    import shutil
    shutil.copy(filepath, "test.p")
    try:
        result = subprocess.run(
            [PARSER_BINARY],
            timeout=10,
            capture_output=True,
            text=True
        )
        out = result.stdout
        if "parse error" in out.lower():
            return ("parse_error", 0.0)
        m = re.search(r":\s*(proved|failed|timeout)\s+in\s+([\d.]+)ms", out)
        if m:
            return (m.group(1), float(m.group(2)))
        return ("unknown", 0.0)
    except subprocess.TimeoutExpired:
        return ("wallclock_timeout", 10000.0)
    except Exception:
        return ("error", 0.0)

def main():
    # Group candidates by domain, filtering by rating as we go
    by_domain = {}
    skipped_hard = 0
    skipped_norating = 0

    for fname in sorted(os.listdir(TPTP_DIR)):
        filepath = os.path.join(TPTP_DIR, fname)
        if not os.path.isfile(filepath):
            continue
        if not is_candidate(filepath):
            continue
        with open(filepath) as f:
            content = f.read()
        if has_includes(content) or has_equality(content):
            continue

        # Filter by difficulty rating
        rating = get_rating(content)
        if rating is None:
            skipped_norating += 1
            continue
        if rating > MAX_RATING:
            skipped_hard += 1
            continue

        # Domain prefix is the alphabetic part (e.g., "SYN" from "SYN001+1.p")
        prefix = ''.join(c for c in os.path.basename(filepath) if c.isalpha())[:3]
        by_domain.setdefault(prefix, []).append(filepath)

    print(f"Filtering summary:")
    print(f"  Skipped (no rating found): {skipped_norating}")
    print(f"  Skipped (rating > {MAX_RATING}): {skipped_hard}")
    print(f"\nCandidates by domain:")
    for d, files in sorted(by_domain.items()):
        print(f"  {d}: {len(files)}")

    # Take an even sample from each domain
    if not by_domain:
        print("\nNo candidates found. Try raising MAX_RATING.")
        return

    per_domain = max(1, MAX_PROBLEMS // len(by_domain))
    candidates = []
    for d, files in sorted(by_domain.items()):
        candidates.extend(files[:per_domain])
    candidates = candidates[:MAX_PROBLEMS]

    print(f"\nRunning {len(candidates)} problems across {len(by_domain)} domains...")

    results = []
    counts = {"proved": 0, "failed": 0, "timeout": 0,
              "wallclock_timeout": 0, "parse_error": 0, "unknown": 0, "error": 0}

    for i, fp in enumerate(candidates, 1):
        name = os.path.basename(fp)
        status, time_ms = run_one(fp)
        counts[status] = counts.get(status, 0) + 1
        results.append((name, status, time_ms))
        if i % 10 == 0:
            print(f"  {i}/{len(candidates)}  ({counts['proved']} proved so far)")

    with open(OUTPUT_FILE, "w") as f:
        f.write(f"# TPTP results: {len(results)} problems\n")
        f.write(f"# Rating filter: <= {MAX_RATING}\n")
        for k, v in counts.items():
            if v > 0:
                f.write(f"# {k}: {v}\n")
        f.write("\n")
        for name, status, time_ms in results:
            f.write(f"{name}\t{status}\t{time_ms:.2f}\n")

    print(f"\nFinal counts:")
    for k, v in counts.items():
        if v > 0:
            print(f"  {k}: {v}")
    print(f"\nWrote {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
