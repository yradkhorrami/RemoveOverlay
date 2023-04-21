#ifndef RemoveOverlay_h
#define RemoveOverlay_h 1
#include <marlin/Processor.h>
#include <marlin/Global.h>
#include "EVENT/LCCollection.h"
#include "IMPL/LCCollectionVec.h"
#include <EVENT/MCParticle.h>
#include "EVENT/ReconstructedParticle.h"
#include "UTIL/LCRelationNavigator.h"
#include "marlin/VerbosityLevels.h"
#include "DDMarlinCED.h"
#include "lcio.h"
#include <vector>

using namespace lcio ;
using namespace marlin ;

class RemoveOverlay : public Processor
{
	public:

		virtual Processor*  newProcessor()
		{
			return new RemoveOverlay;
		}
		RemoveOverlay();
		virtual ~RemoveOverlay() = default;
		RemoveOverlay(const RemoveOverlay&) = delete;
		RemoveOverlay& operator=(const RemoveOverlay&) = delete;
		virtual void init();
		virtual void processEvent( EVENT::LCEvent *pLCEvent );
		EVENT::MCParticle* getMCParticleLinkedToTrack( EVENT::Track* track , LCRelationNavigator TrackMCParticleNav , LCRelationNavigator MCParticleTrackNav );
		EVENT::MCParticle* getMCParticleLinkedToCluster( EVENT::Cluster* cluster , LCRelationNavigator ClusterMCParticleNav , LCRelationNavigator MCParticleClusterNav );
		virtual void check( EVENT::LCEvent *pLCEvent );
		virtual void end();

	private:
		std::string				m_inputPfoCollection{};
		std::string				m_outputPfoCollection{};
		std::string				m_TrackMCTruthLinkCollection{};
		std::string				m_MCTruthTrackLinkCollection{};
		std::string				m_ClusterMCTruthLinkCollection{};
		std::string				m_MCTruthClusterLinkCollection{};
		float					m_MinWeightTrackMCTruthLink;
		float					m_MinWeightClusterMCTruthLink;

};
#endif