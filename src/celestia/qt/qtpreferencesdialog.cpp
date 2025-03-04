// qtpreferencesdialog.cpp
//
// Copyright (C) 2007-2008, Celestia Development Team
// celestia-developers@lists.sourceforge.net
//
// Preferences dialog for Celestia's Qt front-end. Based on
// kdepreferencesdialog.h by Christophe Teyssier.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QFrame>
#include <QCheckBox>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <celengine/body.h>
#include <celengine/location.h>
#include <celengine/render.h>
#include <celestia/celestiacore.h>
#include <celutil/gettext.h>
#include "qtappwin.h"
#include "qtpreferencesdialog.h"


static void SetComboBoxValue(QComboBox* combo, const QVariant& value)
{
    int index;
    int count = combo->count();
    for (index = 0; index < count; ++index)
    {
        if (combo->itemData(index, Qt::UserRole) == value)
        {
            combo->setCurrentIndex(index);
            break;
        }
    }
}


static uint64_t FilterOtherLocations = ~(Location::City |
                                         Location::Observatory |
                                         Location::LandingSite |
                                         Location::Mons |
                                         Location::Mare |
                                         Location::Crater |
                                         Location::Vallis |
                                         Location::Terra |
                                         Location::EruptiveCenter);


PreferencesDialog::PreferencesDialog(QWidget* parent, CelestiaCore* core) :
    QDialog(parent),
    appCore(core)
{
    ui.setupUi(this);

    // Get flags
    Renderer* renderer = appCore->getRenderer();
    Observer* observer = appCore->getSimulation()->getActiveObserver();

    uint64_t renderFlags = renderer->getRenderFlags();
    int orbitMask = renderer->getOrbitMask();
    uint64_t locationFlags = observer->getLocationFilter();
    int labelMode = renderer->getLabelMode();

    ColorTableType colors;
    const ColorTemperatureTable* current = renderer->getStarColorTable();
    if (current->type() == ColorTableType::Blackbody_D65)
    {
        colors = ColorTableType::Blackbody_D65;
    }
    else // if (current->type() == ColorTableType::Enhanced)
    {
        // TODO: Figure out what we should do if we have an unknown color table
        colors = ColorTableType::Enhanced;
    }

    ui.starsCheck->setChecked((renderFlags & Renderer::ShowStars) != 0);
    ui.planetsCheck->setChecked((renderFlags & Renderer::ShowPlanets) != 0);
    ui.dwarfPlanetsCheck->setChecked((renderFlags & Renderer::ShowDwarfPlanets) != 0);
    ui.moonsCheck->setChecked((renderFlags & Renderer::ShowMoons) != 0);
    ui.minorMoonsCheck->setChecked((renderFlags & Renderer::ShowMinorMoons) != 0);
    ui.asteroidsCheck->setChecked((renderFlags & Renderer::ShowAsteroids) != 0);
    ui.cometsCheck->setChecked((renderFlags & Renderer::ShowComets) != 0);
    ui.spacecraftsCheck->setChecked((renderFlags & Renderer::ShowSpacecrafts) != 0);
    ui.galaxiesCheck->setChecked((renderFlags & Renderer::ShowGalaxies) != 0);
    ui.nebulaeCheck->setChecked((renderFlags & Renderer::ShowNebulae) != 0);
    ui.openClustersCheck->setChecked((renderFlags & Renderer::ShowOpenClusters) != 0);
    ui.globularClustersCheck->setChecked((renderFlags & Renderer::ShowGlobulars) != 0);

    ui.atmospheresCheck->setChecked((renderFlags & Renderer::ShowAtmospheres) != 0);
    ui.cloudsCheck->setChecked((renderFlags & Renderer::ShowCloudMaps) != 0);
    ui.cloudShadowsCheck->setChecked((renderFlags & Renderer::ShowCloudShadows) != 0);
    ui.eclipseShadowsCheck->setChecked((renderFlags & Renderer::ShowEclipseShadows) != 0);
    ui.ringShadowsCheck->setChecked((renderFlags & Renderer::ShowRingShadows) != 0);
    ui.planetRingsCheck->setChecked((renderFlags & Renderer::ShowPlanetRings) != 0);
    ui.nightsideLightsCheck->setChecked((renderFlags & Renderer::ShowNightMaps) != 0);
    ui.cometTailsCheck->setChecked((renderFlags & Renderer::ShowCometTails) != 0);
    ui.limitOfKnowledgeCheck->setChecked(observer->getDisplayedSurface() == "limit of knowledge");

    ui.orbitsCheck->setChecked((renderFlags & Renderer::ShowOrbits) != 0);
    ui.fadingOrbitsCheck->setChecked((renderFlags & Renderer::ShowFadingOrbits) != 0);
    ui.starOrbitsCheck->setChecked(orbitMask & Body::Stellar);
    ui.planetOrbitsCheck->setChecked(orbitMask & Body::Planet);
    ui.dwarfPlanetOrbitsCheck->setChecked(orbitMask & Body::DwarfPlanet);
    ui.moonOrbitsCheck->setChecked(orbitMask & Body::Moon);
    ui.minorMoonOrbitsCheck->setChecked(orbitMask & Body::MinorMoon);
    ui.asteroidOrbitsCheck->setChecked(orbitMask & Body::Asteroid);
    ui.cometOrbitsCheck->setChecked(orbitMask & Body::Comet);
    ui.spacecraftOrbitsCheck->setChecked(orbitMask & Body::Spacecraft);
    ui.partialTrajectoriesCheck->setChecked((renderFlags & Renderer::ShowPartialTrajectories) != 0);

    ui.equatorialGridCheck->setChecked((renderFlags & Renderer::ShowCelestialSphere) != 0);
    ui.eclipticGridCheck->setChecked((renderFlags & Renderer::ShowEclipticGrid) != 0);
    ui.galacticGridCheck->setChecked((renderFlags & Renderer::ShowGalacticGrid) != 0);
    ui.horizontalGridCheck->setChecked((renderFlags & Renderer::ShowHorizonGrid) != 0);

    ui.diagramsCheck->setChecked((renderFlags & Renderer::ShowDiagrams) != 0);
    ui.boundariesCheck->setChecked((renderFlags & Renderer::ShowBoundaries) != 0);
    ui.latinNamesCheck->setChecked(!(labelMode & Renderer::I18nConstellationLabels));

    ui.markersCheck->setChecked((renderFlags & Renderer::ShowMarkers) != 0);
    ui.eclipticLineCheck->setChecked((renderFlags & Renderer::ShowEcliptic) != 0);

    ui.starLabelsCheck->setChecked(labelMode & Renderer::StarLabels);
    ui.planetLabelsCheck->setChecked(labelMode & Renderer::PlanetLabels);
    ui.dwarfPlanetLabelsCheck->setChecked(labelMode & Renderer::DwarfPlanetLabels);
    ui.moonLabelsCheck->setChecked(labelMode & Renderer::MoonLabels);
    ui.minorMoonLabelsCheck->setChecked(labelMode & Renderer::MinorMoonLabels);
    ui.asteroidLabelsCheck->setChecked(labelMode & Renderer::AsteroidLabels);
    ui.cometLabelsCheck->setChecked(labelMode & Renderer::CometLabels);
    ui.spacecraftLabelsCheck->setChecked(labelMode & Renderer::SpacecraftLabels);
    ui.galaxyLabelsCheck->setChecked(labelMode & Renderer::GalaxyLabels);
    ui.nebulaLabelsCheck->setChecked(labelMode & Renderer::NebulaLabels);
    ui.openClusterLabelsCheck->setChecked(labelMode & Renderer::OpenClusterLabels);
    ui.globularClusterLabelsCheck->setChecked(labelMode & Renderer::GlobularLabels);
    ui.constellationLabelsCheck->setChecked(labelMode & Renderer::ConstellationLabels);

    ui.locationsCheck->setChecked((labelMode & Renderer::LocationLabels) != 0);
    ui.citiesCheck->setChecked((locationFlags & Location::City) != 0);
    ui.observatoriesCheck->setChecked((locationFlags & Location::Observatory) != 0);
    ui.landingSitesCheck->setChecked((locationFlags & Location::LandingSite) != 0);
    ui.montesCheck->setChecked((locationFlags & Location::Mons) != 0);
    ui.mariaCheck->setChecked((locationFlags & Location::Mare) != 0);
    ui.cratersCheck->setChecked((locationFlags & Location::Crater) != 0);
    ui.vallesCheck->setChecked((locationFlags & Location::Vallis) != 0);
    ui.terraeCheck->setChecked((locationFlags & Location::Terra) != 0);
    ui.volcanoesCheck->setChecked((locationFlags & Location::EruptiveCenter) != 0);
    ui.otherLocationsCheck->setChecked((locationFlags & FilterOtherLocations) != 0);

    int minimumFeatureSize = (int)renderer->getMinimumFeatureSize();
    ui.featureSizeSlider->setValue(minimumFeatureSize);
    ui.featureSizeSpinBox->setValue(minimumFeatureSize);

    ui.renderPathBox->addItem(_("OpenGL 2.1"), 0);

    ui.antialiasLinesCheck->setChecked(renderFlags & Renderer::ShowSmoothLines);

    switch (renderer->getResolution())
    {
        case 0:
            ui.lowResolutionButton->setChecked(true);
            break;

        case 1:
            ui.mediumResolutionButton->setChecked(true);
            break;

        case 2:
            ui.highResolutionButton->setChecked(true);
    }

    auto ambient = static_cast<int>(renderer->getAmbientLightLevel());
    ui.ambientLightSlider->setValue(ambient);
    ui.ambientLightSpinBox->setValue(ambient);

    auto tint = static_cast<int>(renderer->getTintSaturation() * 100.0f);
    ui.tintSaturationSlider->setValue(tint);
    ui.tintSaturationSlider->setEnabled(colors == ColorTableType::Blackbody_D65);
    ui.tintSaturationSpinBox->setValue(tint);
    ui.tintSaturationSpinBox->setEnabled(colors == ColorTableType::Blackbody_D65);

    int starStyle = renderer->getStarStyle();

    switch (starStyle)
    {
        case Renderer::PointStars:
            ui.pointStarsButton->setChecked(true);
            break;

        case Renderer::FuzzyPointStars:
            ui.fuzzyPointStarsButton->setChecked(true);
            break;

        case Renderer::ScaledDiscStars:
            ui.scaledDiscsButton->setChecked(true);
    }

    ui.starColorBox->addItem(_("Blackbody D65"), static_cast<int>(ColorTableType::Blackbody_D65));
    ui.starColorBox->addItem(_("Classic colors"), static_cast<int>(ColorTableType::Enhanced));
    SetComboBoxValue(ui.starColorBox, static_cast<int>(colors));

    ui.autoMagnitudeCheck->setChecked(renderFlags & Renderer::ShowAutoMag);

#ifndef _WIN32
    ui.dateFormatBox->addItem(_("Local format"), astro::Date::Locale);
#endif
    ui.dateFormatBox->addItem(_("Time zone name"), astro::Date::TZName);
    ui.dateFormatBox->addItem(_("UTC offset"), astro::Date::UTCOffset);

    astro::Date::Format dateFormat = appCore->getDateFormat();
    SetComboBoxValue(ui.dateFormatBox, dateFormat);
}


static void setRenderFlag(CelestiaCore* appCore,
                          uint64_t flag,
                          int state)
{
    bool isActive = (state == Qt::Checked);
    Renderer* renderer = appCore->getRenderer();
    uint64_t renderFlags = renderer->getRenderFlags() & ~flag;
    renderer->setRenderFlags(renderFlags | (isActive ? flag : 0));
}


static void setOrbitFlag(CelestiaCore* appCore,
                         int flag,
                         int state)
{
    bool isActive = (state == Qt::Checked);
    Renderer* renderer = appCore->getRenderer();
    int orbitMask = renderer->getOrbitMask() & ~flag;
    renderer->setOrbitMask(orbitMask | (isActive ? flag : 0));
}


static void setLocationFlag(CelestiaCore* appCore,
                            uint64_t flag,
                            int state)
{
    bool isActive = (state == Qt::Checked);
    Observer* observer = appCore->getSimulation()->getActiveObserver();
    uint64_t locationFilter = observer->getLocationFilter() & ~flag;
    observer->setLocationFilter(locationFilter | (isActive ? flag : 0));
}


static void setLabelFlag(CelestiaCore* appCore,
                         int flag,
                         int state)
{
    bool isActive = (state == Qt::Checked);
    Renderer* renderer = appCore->getRenderer();
    int labelMode = renderer->getLabelMode() & ~flag;
    renderer->setLabelMode(labelMode | (isActive ? flag : 0));
}

// Objects

void PreferencesDialog::on_starsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowStars, state);
}


void PreferencesDialog::on_planetsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowPlanets, state);
}


void PreferencesDialog::on_dwarfPlanetsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowDwarfPlanets, state);
}


void PreferencesDialog::on_moonsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowMoons, state);
}


void PreferencesDialog::on_minorMoonsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowMinorMoons, state);
}


void PreferencesDialog::on_asteroidsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowAsteroids, state);
}


void PreferencesDialog::on_cometsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowComets, state);
}


void PreferencesDialog::on_spacecraftsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowSpacecrafts, state);
}


void PreferencesDialog::on_galaxiesCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowGalaxies, state);
}


void PreferencesDialog::on_nebulaeCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowNebulae, state);
}


void PreferencesDialog::on_openClustersCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowOpenClusters, state);
}


void PreferencesDialog::on_globularClustersCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowGlobulars, state);
}


// Features

void PreferencesDialog::on_atmospheresCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowAtmospheres, state);
}


void PreferencesDialog::on_cloudsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowCloudMaps, state);
}


void PreferencesDialog::on_cloudShadowsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowCloudShadows, state);
}


void PreferencesDialog::on_eclipseShadowsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowEclipseShadows, state);
}


void PreferencesDialog::on_ringShadowsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowRingShadows, state);
}


void PreferencesDialog::on_planetRingsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowPlanetRings, state);
}


void PreferencesDialog::on_nightsideLightsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowNightMaps, state);
}


void PreferencesDialog::on_cometTailsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowCometTails, state);
}


void PreferencesDialog::on_limitOfKnowledgeCheck_stateChanged(int state)
{
    Observer* observer = appCore->getSimulation()->getActiveObserver();
    if (state == Qt::Checked)
    {
        observer->setDisplayedSurface("limit of knowledge");
    }
    else
    {
        observer->setDisplayedSurface("");
    }
}


// Orbits

void PreferencesDialog::on_orbitsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowOrbits, state);
}


void PreferencesDialog::on_fadingOrbitsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowFadingOrbits, state);
}


void PreferencesDialog::on_starOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Stellar, state);
}


void PreferencesDialog::on_planetOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Planet, state);
}


void PreferencesDialog::on_dwarfPlanetOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::DwarfPlanet, state);
}


void PreferencesDialog::on_moonOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Moon, state);
}


void PreferencesDialog::on_minorMoonOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::MinorMoon, state);
}


void PreferencesDialog::on_asteroidOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Asteroid, state);
}


void PreferencesDialog::on_cometOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Comet, state);
}


void PreferencesDialog::on_spacecraftOrbitsCheck_stateChanged(int state)
{
    setOrbitFlag(appCore, Body::Spacecraft, state);
}


void PreferencesDialog::on_partialTrajectoriesCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowPartialTrajectories, state);
}


// Grids

void PreferencesDialog::on_equatorialGridCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowCelestialSphere, state);
}


void PreferencesDialog::on_eclipticGridCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowEclipticGrid, state);
}


void PreferencesDialog::on_galacticGridCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowGalacticGrid, state);
}


void PreferencesDialog::on_horizontalGridCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowHorizonGrid, state);
}


// Constellations

void PreferencesDialog::on_diagramsCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowDiagrams, state);
}


void PreferencesDialog::on_boundariesCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowBoundaries, state);
}


void PreferencesDialog::on_latinNamesCheck_stateChanged(int state)
{
    // "Latin Names" Checkbox has inverted meaning
    state = state == Qt::Checked ? Qt::Unchecked : Qt::Checked;
    setLabelFlag(appCore, Renderer::I18nConstellationLabels, state);
}


// Other guides

void PreferencesDialog::on_markersCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowMarkers, state);
}


void PreferencesDialog::on_eclipticLineCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowEcliptic, state);
}



// Labels

void PreferencesDialog::on_starLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::StarLabels, state);
}


void PreferencesDialog::on_planetLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::PlanetLabels, state);
}


void PreferencesDialog::on_dwarfPlanetLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::DwarfPlanetLabels, state);
}


void PreferencesDialog::on_moonLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::MoonLabels, state);
}


void PreferencesDialog::on_minorMoonLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::MinorMoonLabels, state);
}


void PreferencesDialog::on_asteroidLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::AsteroidLabels, state);
}


void PreferencesDialog::on_cometLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::CometLabels, state);
}


void PreferencesDialog::on_spacecraftLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::SpacecraftLabels, state);
}


void PreferencesDialog::on_galaxyLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::GalaxyLabels, state);
}


void PreferencesDialog::on_nebulaLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::NebulaLabels, state);
}


void PreferencesDialog::on_openClusterLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::OpenClusterLabels, state);
}


void PreferencesDialog::on_globularClusterLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::GlobularLabels, state);
}


void PreferencesDialog::on_constellationLabelsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::ConstellationLabels, state);
}


// Locations

void PreferencesDialog::on_locationsCheck_stateChanged(int state)
{
    setLabelFlag(appCore, Renderer::LocationLabels, state);
}


void PreferencesDialog::on_citiesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::City, state);
}


void PreferencesDialog::on_observatoriesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Observatory, state);
}


void PreferencesDialog::on_landingSitesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::LandingSite, state);
}


void PreferencesDialog::on_montesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Mons, state);
}


void PreferencesDialog::on_mariaCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Mare, state);
}


void PreferencesDialog::on_cratersCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Crater, state);
}


void PreferencesDialog::on_vallesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Vallis, state);
}


void PreferencesDialog::on_terraeCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::Terra, state);
}


void PreferencesDialog::on_volcanoesCheck_stateChanged(int state)
{
    setLocationFlag(appCore, Location::EruptiveCenter, state);
}


void PreferencesDialog::on_otherLocationsCheck_stateChanged(int state)
{
    setLocationFlag(appCore, FilterOtherLocations, state);
}


void PreferencesDialog::on_featureSizeSlider_valueChanged(int value)
{
    Renderer* renderer = appCore->getRenderer();
    renderer->setMinimumFeatureSize(static_cast<float>(value));
    ui.featureSizeSpinBox->blockSignals(true);
    ui.featureSizeSpinBox->setValue(value);
    ui.featureSizeSpinBox->blockSignals(false);
}


void PreferencesDialog::on_featureSizeSpinBox_valueChanged(int value)
{
    Renderer* renderer = appCore->getRenderer();
    renderer->setMinimumFeatureSize(static_cast<float>(value));
    ui.featureSizeSlider->blockSignals(true);
    ui.featureSizeSlider->setValue(value);
    ui.featureSizeSlider->blockSignals(false);
}


void PreferencesDialog::on_renderPathBox_currentIndexChanged(int /*index*/)
{
}


void PreferencesDialog::on_antialiasLinesCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowSmoothLines, state);
}


// Texture resolution

void PreferencesDialog::on_lowResolutionButton_clicked()
{
    if (ui.lowResolutionButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setResolution(0);
    }
}


void PreferencesDialog::on_mediumResolutionButton_clicked()
{
    if (ui.mediumResolutionButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setResolution(1);
    }
}


void PreferencesDialog::on_highResolutionButton_clicked()
{
    if (ui.highResolutionButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setResolution(2);
    }
}


// Ambient light

void PreferencesDialog::on_ambientLightSlider_valueChanged(int value)
{
    float ambient = static_cast<float>(value) / 100.0f;
    appCore->getRenderer()->setAmbientLightLevel(ambient);
    auto savedBlockState = ui.ambientLightSpinBox->blockSignals(true);
    ui.ambientLightSpinBox->setValue(value);
    ui.ambientLightSpinBox->blockSignals(savedBlockState);
}


void PreferencesDialog::on_ambientLightSpinBox_valueChanged(int value)
{
    float ambient = static_cast<float>(value) / 100.0f;
    appCore->getRenderer()->setAmbientLightLevel(ambient);
    auto savedBlockState = ui.ambientLightSlider->blockSignals(true);
    ui.ambientLightSlider->setValue(value);
    ui.ambientLightSlider->blockSignals(savedBlockState);
}


// Tint saturation

void PreferencesDialog::on_tintSaturationSlider_valueChanged(int value)
{
    float tintSaturation = static_cast<float>(value) / 100.0f;
    appCore->getRenderer()->setTintSaturation(tintSaturation);
    auto savedBlockState = ui.tintSaturationSpinBox->blockSignals(true);
    ui.tintSaturationSpinBox->setValue(value);
    ui.tintSaturationSpinBox->blockSignals(savedBlockState);
}


void PreferencesDialog::on_tintSaturationSpinBox_valueChanged(int value)
{
    float tintSaturation = static_cast<float>(value) / 100.0f;
    appCore->getRenderer()->setTintSaturation(tintSaturation);
    auto savedBlockState = ui.tintSaturationSlider->blockSignals(true);
    ui.tintSaturationSlider->setValue(value);
    ui.tintSaturationSlider->blockSignals(savedBlockState);
}


// Star style

void PreferencesDialog::on_pointStarsButton_clicked()
{
    if (ui.pointStarsButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setStarStyle(Renderer::PointStars);
    }
}


void PreferencesDialog::on_scaledDiscsButton_clicked()
{
    if (ui.scaledDiscsButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setStarStyle(Renderer::ScaledDiscStars);
    }
}


void PreferencesDialog::on_fuzzyPointStarsButton_clicked()
{
    if (ui.fuzzyPointStarsButton->isChecked())
    {
        Renderer* renderer = appCore->getRenderer();
        renderer->setStarStyle(Renderer::FuzzyPointStars);
    }
}


void PreferencesDialog::on_autoMagnitudeCheck_stateChanged(int state)
{
    setRenderFlag(appCore, Renderer::ShowAutoMag, state);
}


// Star colors

void PreferencesDialog::on_starColorBox_currentIndexChanged(int index)
{
    Renderer* renderer = appCore->getRenderer();
    QVariant itemData = ui.starColorBox->itemData(index, Qt::UserRole);
    ColorTableType value = static_cast<ColorTableType>(itemData.toInt());
    renderer->setStarColorTable(GetStarColorTable(value));
    bool enableTintSaturation = value == ColorTableType::Blackbody_D65;
    ui.tintSaturationSlider->setEnabled(enableTintSaturation);
    ui.tintSaturationSpinBox->setEnabled(enableTintSaturation);
}


// Time

void PreferencesDialog::on_dateFormatBox_currentIndexChanged(int index)
{
    QVariant itemData = ui.dateFormatBox->itemData(index, Qt::UserRole);
    astro::Date::Format dateFormat = (astro::Date::Format) itemData.toInt();
    appCore->setDateFormat(dateFormat);
}
