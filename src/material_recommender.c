#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LINE 2048
#define MAX_ALLOY 25.0

typedef struct {
    char name[100];
    int cluster;
    double Cr, Ni, Mo, Mn, Si, V, Nb, Ti;
    double yield;
    double tensile;
    double ductility;
} Material;

double alloy_index(Material m) {
    return m.Cr + m.Ni + m.Mo + m.Mn + m.Si + m.V + m.Nb + m.Ti;
}

double thermal_score(Material m) {
    double s = alloy_index(m) / MAX_ALLOY;
    if (m.cluster >= 6) s += 0.2;
    return fmin(s, 1.0);
}

double electrical_score(Material m) {
    double s = 1.0 - (alloy_index(m) / MAX_ALLOY);
    if (m.cluster >= 6) s -= 0.2;
    return fmax(s, 0.0);
}

double cost_score(Material m) {
    return fmax(1.0 - alloy_index(m) / MAX_ALLOY, 0.0);
}

double fatigue_score(Material m) {
    double y = fmin(m.yield / 1500.0, 1.0);
    double d = fmin(m.ductility / 50.0, 1.0);
    return 0.6 * y + 0.4 * d;
}

int main() {
    FILE *fp = fopen("data/steel_dataset.csv.csv", "r");
    if (!fp) {
        printf("Cannot find dataset file.\n");
        return 1;
    }

    double req_yield, w_t, w_e, w_c;
    printf("Required Yield Strength (MPa): ");
    scanf("%lf", &req_yield);

    printf("Thermal importance (0–1): ");
    scanf("%lf", &w_t);

    printf("Electrical importance (0–1): ");
    scanf("%lf", &w_e);

    printf("Cost importance (0–1): ");
    scanf("%lf", &w_c);

    char line[MAX_LINE];
    fgets(line, MAX_LINE, fp);  // skip header

    Material best;
    double best_score = -1.0;

    while (fgets(line, MAX_LINE, fp)) {
        Material m = {0};

        sscanf(line,
            "%[^,],%*[^,],%d,%*[^,],%*[^,],"
            "%lf,%*lf,%lf,%*lf,%lf,%lf,%*lf,%*lf,%*lf,%*lf,%lf,%lf,%lf,%*lf,%lf,%*lf,%lf,%*lf,%lf,%*lf,%lf,%lf,%lf",
            m.name, &m.cluster,
            &m.Cr, &m.Mn, &m.Ni, &m.Ti, &m.Si, &m.V, &m.Mo, &m.Nb,
            &m.yield, &m.tensile, &m.ductility
        );

        double strength = fmin(m.yield / req_yield, 1.0);

        double score =
            0.30 * strength +
            0.20 * thermal_score(m) * w_t +
            0.15 * electrical_score(m) * w_e +
            0.15 * cost_score(m) * w_c +
            0.20 * fatigue_score(m);

        if (score > best_score) {
            best_score = score;
            best = m;
        }
    }

    fclose(fp);

    printf("\n=== BEST MATERIAL ===\n");
    printf("Name: %s\n", best.name);
    printf("Yield Strength: %.1f MPa\n", best.yield);
    printf("Ductility: %.1f %%\n", best.ductility);
    printf("Cluster: %d\n", best.cluster);
    printf("Score: %.3f\n", best_score);

    return 0;
}
