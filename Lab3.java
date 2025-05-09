
import java.util.*;

public class Lab3 {

// ========================== CLASS: CLAUSE ========================== 
    public static class Clause {

        public Set<String> p; // Positive literals (A means A is true)
        public Set<String> n; // Negative literals (¬A is stored as "A" in this set)

        // Constructor that takes two sets: positive & negative literal
        public Clause(Set<String> p, Set<String> n) {
            this.p = new HashSet<>(p);      // HashSet<>() : No Duplicates     
            this.n = new HashSet<>(n);
        }

        // Default constructor (when no params are set)
        public Clause() {
            this.p = new HashSet<>();
            this.n = new HashSet<>();
        }

        // Return true if clause is tautology: contains both a literal and its negation
        // For every literal in positive --> if there is the same literal in negative --> return true
        // Note: ¬literal is stored as "literal" in this set
        public boolean isTautology() {
            for (String literal : p) {
                if (n.contains(literal)) {
                    return true;
                }
            }
            return false;
        }

        // To check if this clause subsumes another
        public boolean subsumes(Clause other) {
            // This ⪯ other: this.p ⊆ other.p && this.n ⊆ other.n
            return other.p.containsAll(this.p) && other.n.containsAll(this.n);
        }

        // Printout for clause
        @Override
        public String toString() {
            return "Pos: " + p + ", Neg: " + n;
        }
        // Equality and hashCode to allow using in sets/maps
        @Override
        public boolean equals(Object o) {
            if (this == o) {
                return true;
            }
            if (!(o instanceof Clause)) {
                return false;
            }
            Clause c = (Clause) o;
            return c.p.equals(c.p) && n.equals(c.n);
        }

        @Override
        public int hashCode() {
            return Objects.hash(p, n);
        }
    }

// ========================== FUNCTION: RESOLUTION ==========================
    public static Clause Resolution(Clause A, Clause B) {
        /*
    if A.p ∩ B.n = {} and A.n ∩ B.p = {} do
        return false
         */

        // A_p = A.p ∩ B.n
        Set<String> A_p = new HashSet<>(A.p);
        A_p.retainAll(B.n);

        // A_n = A.n ∩ B.p 
        Set<String> A_n = new HashSet<>(A.n);
        A_n.retainAll(B.p);

        if (A_p.isEmpty() && A_n.isEmpty()) {
            return null;
        }

        /*
    if (A.p ∩ B.n) ̸= {} do
        a ← Pick random element from(A.p ∩ B.n)
        A.p ← A.p − {a}
        B.n ← B.n − {a}
         */
        String a;                              // Literal we'll resolve on
        Set<String> C_p = new HashSet<>(A.p);  // Start with A.p (will become C.p)
        Set<String> C_n = new HashSet<>(A.n);  // Start with A.n (will become C.n)

        if (!A_p.isEmpty()) {
            a = A_p.iterator().next();           // Pick a literal from A.p ∩ B.n
            C_p.remove(a);                       // A.p − {a}

            Set<String> B_n = new HashSet<>(B.n); // Copy B.n so we can safely remove a
            B_n.remove(a);                        // B.n − {a}

            C_p.addAll(B.p);                     // C.p = (A.p − {a}) ∪ B.p
            C_n.addAll(B_n);                     // C.n = A.n ∪ (B.n − {a})
        } else {
            a = A_n.iterator().next();           // Pick a literal from A.n ∩ B.p
            C_n.remove(a);                       // A.n − {a}

            Set<String> B_p = new HashSet<>(B.p); // Copy B.p so we can safely modify
            B_p.remove(a);                        // B.p − {a}

            C_p.addAll(B_p);                     // C.p = A.p ∪ (B.p − {a})
            C_n.addAll(B.n);                     // C.n = (A.n − {a}) ∪ B.n
        }

        Clause result = new Clause(C_p, C_n);    // C.p ∩ C.n

        if (result.isTautology()) {
            return null;
        } else {
            return result;
        }
        // No need to explicitly remove duplicates cause HashSet don't allow duplicates.
    }

// ========================== FUNCTION: SOLVER ==========================
    public static Set<Clause> Solver(Set<Clause> KB) {
        // KB ← Incorporate(KB, {})
        Set<Clause> currentKB = incorporate(new HashSet<>(), KB);

        // repeat
        while (true) {
            Set<Clause> S = new HashSet<>();                        // S = {}
            List<Clause> clauseList = new ArrayList<>(currentKB);   // KB′ ← KB

            // for each A, B in KB :
            for (int i = 0; i < clauseList.size(); i++) {
                for (int j = i + 1; j < clauseList.size(); j++) {

                    Clause C = Resolution(clauseList.get(i), clauseList.get(j));   // C ← Resolution(A,B)

                    if (C != null) { // if C ̸= false do
                        S.add(C);   // S ← S ∪ {C}
                    }
                }
            }

            // If no new clause, return KB
            if (S.isEmpty()) {       // if S = {}
                return currentKB;   // return KB
            }

            // Incorporate new clauses
            Set<Clause> newKB = incorporate(S, currentKB);  // KB ← Incorporate(S, KB)

            // until KB' = KB (EXIT)
            if (newKB.equals(currentKB)) {
                return currentKB;
            }

            currentKB = newKB;
        }
    }

// ========================== FUNCTION: INCORPORATE ==========================
    public static Set<Clause> incorporate(Set<Clause> S, Set<Clause> KB) {
        Set<Clause> newKB = new HashSet<>(KB);

        for (Clause A : S) {                        // for each A in S :
            newKB = Incorporate_Clause(A, newKB);   //      KB ← Incorporate clause(A, KB)
        }
        return newKB;                               // return KB
    }

// ========================== FUNCTION: INCORPORATE CLAUSE ==========================
    public static Set<Clause> Incorporate_Clause(Clause A, Set<Clause> KB) {

        for (Clause B : KB) {        // for each B in KB :
            if (B.subsumes(A)) {     // if B ⪯ A do
                return KB;          // return KB
            }
        }

        // Remove clauses that A subsumes
        Set<Clause> newKB = new HashSet<>(KB);
        for (Clause B : KB) {        // for each B in KB :
            if (A.subsumes(B)) {      // if A ⪯ B do
                newKB.remove(B);    // KB ← KB − {B}
            }
        }

        newKB.add(A);   // KB ← KB ∪ {A}
        return newKB;   // return KB
    }

// ========================== MAIN ==========================
    public static void main(String[] args) {
        Set<Clause> KB = new HashSet<>();

        // ======== According to 2. under "Drawing Conclusions" in "solver for clauses in CNF.pdf" ========
        // ¬sun ∨ ¬money ∨ ice
        KB.add(new Clause(Set.of("ice"), Set.of("sun", "money")));

        // ¬money ∨ ice ∨ movie
        KB.add(new Clause(Set.of("ice", "movie"), Set.of("money")));

        // ¬movie ∨ money
        KB.add(new Clause(Set.of("money"), Set.of("movie")));

        // ¬movie ∨ ¬ice
        KB.add(new Clause(Set.of(), Set.of("movie", "ice")));

        // movie
        KB.add(new Clause(Set.of("movie"), Set.of()));

        // sun ∨ money ∨ cry
        KB.add(new Clause(Set.of("sun", "money", "cry"), Set.of()));

        // Run the solver
        Set<Clause> result = Solver(KB);

        // Print the final knowledge base
        System.out.println("Derived Knowledge Base:");
        for (Clause clause : result) {
            System.out.println(clause);
        }
    }
}
