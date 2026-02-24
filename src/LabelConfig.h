// Label Configuration for UAH Banknote Recognition
// Цей файл містить розпізнавані номіналу українських гривні

#ifndef _LABEL_CONFIG_H_
#define _LABEL_CONFIG_H_

// Підтримувані номіналу гривні
enum UAHBanknote {
    UAH_5 = 5,
    UAH_10 = 10,
    UAH_20 = 20,
    UAH_50 = 50,
    UAH_100 = 100,
    UAH_200 = 200,
    UAH_500 = 500,
    UAH_1000 = 1000,
    UAH_UNKNOWN = 0
};

// Конвертація строки до номіналу
/*
static UAHBanknote parseUAHLabel(const char* label) {
    if (!label) return UAH_UNKNOWN;
    
    if (strcmp(label, "5_UAH") == 0) return UAH_5;
    if (strcmp(label, "10_UAH") == 0) return UAH_10;
    if (strcmp(label, "20_UAH") == 0) return UAH_20;
    if (strcmp(label, "50_UAH") == 0) return UAH_50;
    if (strcmp(label, "100_UAH") == 0) return UAH_100;
    if (strcmp(label, "200_UAH") == 0) return UAH_200;
    if (strcmp(label, "500_UAH") == 0) return UAH_500;
    if (strcmp(label, "1000_UAH") == 0) return UAH_1000;
    
    return UAH_UNKNOWN;
}
*/

// Усі можливі labels для розпізнавання
static const char* VALID_LABELS[] = {
    "5_UAH", "10_UAH", "20_UAH", "50_UAH",
    "100_UAH", "200_UAH", "500_UAH", "1000_UAH"
};

// Кількість підтримуваних номіналів
static const int NUM_LABELS = 8;

#endif
