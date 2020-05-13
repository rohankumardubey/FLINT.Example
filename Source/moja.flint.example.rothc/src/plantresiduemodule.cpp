#include "moja/modules/fullcam/plantresiduemodule.h"

#include "moja/flint/variable.h"

#include "moja/timeseries.h"
#include "moja/notificationcenter.h"
#include "moja/signals.h"
#include "moja/flint/ioperation.h"

namespace moja {
namespace modules {
namespace fullcam {

void PlantResidueModule::configure(const DynamicObject& config) { }

void PlantResidueModule::subscribe(NotificationCenter& notificationCenter) {
	notificationCenter.subscribe(signals::TimingInit, &PlantResidueModule::onTimingInit, *this);
	notificationCenter.subscribe(signals::TimingStep, &PlantResidueModule::onTimingStep, *this);
}

void PlantResidueModule::onTimingInit() {
	_soilDPM = _landUnitData->getPool("SoilDPM");
	_soilRPM = _landUnitData->getPool("SoilRPM");
	_plantCM = _landUnitData->getPool("plantCM");
	_presCM = _landUnitData->getVariable("presCM");

	const auto soil = _landUnitData->getVariable("soil")->value();
	double dToRRatioInPres = soil["dToRRatioInPres"];
	//auto dToRRatioInPres = _landUnitData->getObject("soil")->getField("dToRRatioInPres").Double();
	_dFracPres = dToRRatioInPres / (1.0 + dToRRatioInPres);
	_rFracPres = 1.0 - _dFracPres;
}

void PlantResidueModule::onTimingStep() {
	const auto& presCMTimeSeries = _presCM->value().extract<TimeSeries>();
	double presCM = presCMTimeSeries.value();

	if (presCM > 0.0) {
		auto operation = _landUnitData->createStockOperation();
		operation
			->addTransfer(_plantCM, _soilDPM, presCM * _dFracPres)
			->addTransfer(_plantCM, _soilRPM, presCM * _rFracPres);
		_landUnitData->submitOperation(operation);
	}

}
}
}
} // namespace moja::modules::fullcam

