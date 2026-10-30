#pragma once
#include <QString>
class QWidget;
void applyUiStudyPearl(QWidget* window);
void applyUiStudyGraphite(QWidget* window);
void applyUiStudyFolio(QWidget* window);
void applyUiStudy(QWidget* window,const QString& id);
bool renderUiStudies(const QString& directory);
void showStudioAppearance(QWidget* window);
