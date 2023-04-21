#include "RemoveOverlay.h"
RemoveOverlay aRemoveOverlay;

RemoveOverlay::RemoveOverlay() :

Processor("RemoveOverlay"),
m_MinWeightTrackMCTruthLink(0.5),
m_MinWeightClusterMCTruthLink(0.5)
{
	_description = "Finds PFOs that are linked to overlay MCParticles and removes from PFOs collection to be clustered later in jets";

	registerInputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
								"inputPfoCollection",
								"Name of input pfo collection",
								m_inputPfoCollection,
								std::string("PandoraPFOs")
							);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
								"outputPfoCollection",
								"Name of output pfo collection",
								m_outputPfoCollection,
								std::string("PandoraPFOsWithoutOverlay")
							);

	registerInputCollection(	LCIO::LCRELATION,
								"TrackMCTruthLinkCollection",
								"Name of input TrackMCTruthLink Collection",
								m_TrackMCTruthLinkCollection,
								std::string("MarlinTrkTracksMCTruthLink")
							);

	registerInputCollection(	LCIO::LCRELATION,
								"MCTruthTrackLinkCollection",
								"Name of input MCTruthTrackLink Collection",
								m_MCTruthTrackLinkCollection,
								std::string("MCTruthMarlinTrkTracksLink")
							);

	registerInputCollection(	LCIO::LCRELATION,
								"ClusterMCTruthLinkCollection",
								"Name of input ClusterMCTruthLink Collection",
								m_ClusterMCTruthLinkCollection,
								std::string("MarlinTrkClustersMCTruthLink")
							);

	registerInputCollection(	LCIO::LCRELATION,
								"MCTruthClusterLinkCollection",
								"Name of input MCTruthClusterLink Collection",
								m_MCTruthClusterLinkCollection,
								std::string("MCTruthMarlinTrkClustersLink")
							);

	registerProcessorParameter(	"MinWeightTrackMCTruthLink" ,
								"Minimum acceptable weight for Track <---> MCParticle Link"  ,
								m_MinWeightTrackMCTruthLink ,
								float(0.9f)
							);

	registerProcessorParameter(	"MinWeightClusterMCTruthLink" ,
								"Minimum acceptable weight for Cluster <---> MCParticle Link"  ,
								m_MinWeightClusterMCTruthLink ,
								float(0.9f)
							);

}

void RemoveOverlay::init()
{
	streamlog_out(DEBUG) << "	init called  " << std::endl;
	printParameters();
}

void RemoveOverlay::processEvent( EVENT::LCEvent *pLCEvent )
{

	LCCollection *inputPfoCollection{};
	IMPL::LCCollectionVec* outputPfoCollection(NULL);
	outputPfoCollection = new IMPL::LCCollectionVec( LCIO::RECONSTRUCTEDPARTICLE );
	outputPfoCollection->setSubset( true );
	int n_PFO = -1;
	streamlog_out(MESSAGE) << "" << std::endl;
	streamlog_out(MESSAGE) << "		////////////////////////////////////////////////////////////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "		////////////////////	Processing event 	" << pLCEvent->getEventNumber() << "	////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "		////////////////////////////////////////////////////////////////////////////" << std::endl;

	try
	{
		LCRelationNavigator TrackMCParticleNav( pLCEvent->getCollection( m_TrackMCTruthLinkCollection ) );
		LCRelationNavigator MCParticleTrackNav( pLCEvent->getCollection( m_MCTruthTrackLinkCollection ) );
		LCRelationNavigator ClusterMCParticleNav( pLCEvent->getCollection( m_ClusterMCTruthLinkCollection ) );
		LCRelationNavigator MCParticleClusterNav( pLCEvent->getCollection( m_MCTruthClusterLinkCollection ) );

		inputPfoCollection = pLCEvent->getCollection( m_inputPfoCollection );
		n_PFO = inputPfoCollection->getNumberOfElements();
		if ( n_PFO == -1 ) streamlog_out(DEBUG9) << "		Input PFO collection (" << m_inputPfoCollection << ") has no element (PFO) " << std::endl;
		streamlog_out(DEBUG9) << "		Total Number of PFOs: " << n_PFO << std::endl;
		bool PFOisOverlay = false;
		for (int i_pfo = 0; i_pfo < n_PFO ; ++i_pfo)
		{
			PFOisOverlay = false;
			ReconstructedParticle *pfo = dynamic_cast<ReconstructedParticle*>( inputPfoCollection->getElementAt( i_pfo ) );
			streamlog_out(DEBUG8) << "" << std::endl;
			streamlog_out(DEBUG8) << "		-------------------------------------------------------" << std::endl;
			streamlog_out(DEBUG8) << "		Processing PFO at index " << i_pfo << std::endl;
			streamlog_out(DEBUG5) << *pfo << std::endl;
			for ( unsigned int i_trk = 0 ; i_trk < ( pfo->getTracks() ).size() ; ++i_trk )
			{
				Track *track = pfo->getTracks()[ i_trk ];
				MCParticle *linkedMCP = getMCParticleLinkedToTrack( track , TrackMCParticleNav , MCParticleTrackNav );
				if ( linkedMCP != NULL )
				{
					streamlog_out(DEBUG6) << "		Found a linked MCParticle to track" << std::endl;
					streamlog_out(DEBUG5) << *linkedMCP << std::endl;
					if ( linkedMCP->isOverlay() ) PFOisOverlay = true;
				}
				else
				{
					streamlog_out(DEBUG6) << "		Track is not linked to a MCParticle" << std::endl;
				}
			}
			for ( unsigned int i_clu = 0 ; i_clu < ( pfo->getClusters() ).size() ; ++i_clu )
			{
				Cluster *cluster = pfo->getClusters()[ i_clu ];
				MCParticle *linkedMCP = getMCParticleLinkedToCluster( cluster , ClusterMCParticleNav , MCParticleClusterNav );
				if ( linkedMCP != NULL )
				{
					streamlog_out(DEBUG6) << "		Found a linked MCParticle to cluster" << std::endl;
					streamlog_out(DEBUG5) << *linkedMCP << std::endl;
					if ( linkedMCP->isOverlay() ) PFOisOverlay = true;
				}
				else
				{
					streamlog_out(DEBUG6) << "		Cluster is not linked to a MCParticle" << std::endl;
				}
			}
			if ( !PFOisOverlay )
			{
				streamlog_out(DEBUG7) << "		********** PFO AT INDEX " << i_pfo << " IS NOT OVERLAY **********" << std::endl;
				outputPfoCollection->addElement( pfo );
			}
			else
			{
				streamlog_out(DEBUG7) << "		********** PFO AT INDEX " << i_pfo << " IS OVERLAY **********" << std::endl;
			}
		}
		pLCEvent->addCollection( outputPfoCollection , m_outputPfoCollection );
	}
	catch(DataNotAvailableException &e)
	{
		streamlog_out(MESSAGE) << "Input collection not found in event " << pLCEvent->getEventNumber() << std::endl;
	}

}

EVENT::MCParticle* RemoveOverlay::getMCParticleLinkedToTrack( EVENT::Track* track , LCRelationNavigator TrackMCParticleNav , LCRelationNavigator MCParticleTrackNav )
{
	MCParticle *linkedMCP = NULL;
	const EVENT::LCObjectVec& mcpvec = TrackMCParticleNav.getRelatedToObjects( track );
	const EVENT::FloatVec&  mcpweightvec = TrackMCParticleNav.getRelatedToWeights( track );
	double maxweightTRKtoMCP = 0.;
	double maxweightMCPtoTRK = 0.;
	int iTRKtoMCPmax = -1;
	for ( unsigned int i_mcp = 0; i_mcp < mcpvec.size(); i_mcp++ )
	{
		double mcp_weight = mcpweightvec.at( i_mcp );
		MCParticle *testMCP = (MCParticle *) mcpvec.at( i_mcp );
		if ( mcp_weight > maxweightTRKtoMCP && mcp_weight >= m_MinWeightTrackMCTruthLink )
		{
			maxweightTRKtoMCP = mcp_weight;
			iTRKtoMCPmax = i_mcp;
			streamlog_out(DEBUG0) << "	MCParticle at index: " << i_mcp << " has PDG: " << testMCP->getPDG() << " and MCParticle to Track link weight is " << mcp_weight << std::endl;
		}
	}
	if ( iTRKtoMCPmax != -1 )
	{
		MCParticle *testMCP = (MCParticle *) mcpvec.at( iTRKtoMCPmax );
		streamlog_out(DEBUG0) << "	MCParticle at index: " << iTRKtoMCPmax << " with PDG code: " << testMCP->getPDG() << " has highest link weight to input track" << std::endl;
		const EVENT::LCObjectVec& trkvec = MCParticleTrackNav.getRelatedToObjects( testMCP );
		const EVENT::FloatVec&  trkweightvec = MCParticleTrackNav.getRelatedToWeights( testMCP );
		int iMCPtoTRKmax = -1;
		for ( unsigned int i_trk = 0; i_trk < trkvec.size(); i_trk++ )
		{
			double track_weight = trkweightvec.at( i_trk );
			if ( track_weight > maxweightMCPtoTRK && track_weight >= m_MinWeightTrackMCTruthLink )
			{
				maxweightMCPtoTRK = track_weight;
				iMCPtoTRKmax = i_trk;
				streamlog_out(DEBUG0) << "	Track at index: " << i_trk << " has highest link weight to MCParticle = " << track_weight << std::endl;
			}
		}
		if ( iMCPtoTRKmax != -1 )
		{
			Track *linkedTrack = (Track *) trkvec.at( iMCPtoTRKmax );
			if ( linkedTrack == track )
			{
				streamlog_out(DEBUG0) << "	Track linked to MCParticle with highest weight" << std::endl;
				linkedMCP = (MCParticle *) mcpvec.at(iTRKtoMCPmax);
			}
		}
	}
	return linkedMCP;
}

EVENT::MCParticle* RemoveOverlay::getMCParticleLinkedToCluster( EVENT::Cluster* cluster , LCRelationNavigator ClusterMCParticleNav , LCRelationNavigator MCParticleClusterNav )
{
	MCParticle *linkedMCP = NULL;
	const EVENT::LCObjectVec& mcpvec = ClusterMCParticleNav.getRelatedToObjects( cluster );
	const EVENT::FloatVec&  mcpweightvec = ClusterMCParticleNav.getRelatedToWeights( cluster );
	double maxweightCLUtoMCP = 0.;
	double maxweightMCPtoCLU = 0.;
	int iCLUtoMCPmax = -1;
	for ( unsigned int i_mcp = 0; i_mcp < mcpvec.size(); i_mcp++ )
	{
		double mcp_weight = mcpweightvec.at( i_mcp );
		MCParticle *testMCP = (MCParticle *) mcpvec.at( i_mcp );
		if ( mcp_weight > maxweightCLUtoMCP && mcp_weight >= m_MinWeightClusterMCTruthLink )
		{
			maxweightCLUtoMCP = mcp_weight;
			iCLUtoMCPmax = i_mcp;
			streamlog_out(DEBUG0) << "	MCParticle at index: " << i_mcp << " has PDG: " << testMCP->getPDG() << " and MCParticle to Cluster link weight is " << mcp_weight << std::endl;
		}
	}
	if ( iCLUtoMCPmax != -1 )
	{
		MCParticle *testMCP = (MCParticle *) mcpvec.at( iCLUtoMCPmax );
		streamlog_out(DEBUG0) << "	MCParticle at index: " << iCLUtoMCPmax << " with PDG code: " << testMCP->getPDG() << " has highest link weight to input cluster" << std::endl;
		const EVENT::LCObjectVec& cluvec = MCParticleClusterNav.getRelatedToObjects( testMCP );
		const EVENT::FloatVec&  cluweightvec = MCParticleClusterNav.getRelatedToWeights( testMCP );
		int iMCPtoCLUmax = -1;
		for ( unsigned int i_clu = 0; i_clu < cluvec.size(); i_clu++ )
		{
			double cluster_weight = cluweightvec.at( i_clu );
			if ( cluster_weight > maxweightMCPtoCLU && cluster_weight >= m_MinWeightClusterMCTruthLink )
			{
				maxweightMCPtoCLU = cluster_weight;
				iMCPtoCLUmax = i_clu;
				streamlog_out(DEBUG0) << "	Cluster at index: " << i_clu << " has highest link weight to MCParticle = " << cluster_weight << std::endl;
			}
		}
		if ( iMCPtoCLUmax != -1 )
		{
			Cluster *linkedCluster = (Cluster *) cluvec.at( iMCPtoCLUmax );
			if ( linkedCluster == cluster )
			{
				streamlog_out(DEBUG0) << "	Cluster linked to MCParticle with highest weight" << std::endl;
				linkedMCP = (MCParticle *) mcpvec.at(iCLUtoMCPmax);
			}
		}
	}
	return linkedMCP;
}

void RemoveOverlay::check( EVENT::LCEvent *pLCEvent )
{
	LCCollection *inputPfoCollection{};
	LCCollection *outputPfoCollection{};
	try
	{
		inputPfoCollection = pLCEvent->getCollection( m_inputPfoCollection );
		outputPfoCollection = pLCEvent->getCollection( m_outputPfoCollection );
		int n_inputPFOs = inputPfoCollection->getNumberOfElements();
		int n_outputPFOs = outputPfoCollection->getNumberOfElements();
		streamlog_out(MESSAGE) << "	CHECK : processed event: " << pLCEvent->getEventNumber() << " (Number of inputPFOS: " << n_inputPFOs << " , Number of outputPFOs: " << n_outputPFOs <<")" << std::endl;
	}
	catch(DataNotAvailableException &e)
        {
          streamlog_out(MESSAGE) << "	Input/Output collection not found in event " << pLCEvent->getEventNumber() << std::endl;
        }
}
void RemoveOverlay::end()
{

	streamlog_out(MESSAGE) << " " << std::endl;
	streamlog_out(MESSAGE) << "	////////////////////////////////////////////////////////////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "	////////////////////////////////////////////////////////////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "	///////////////////////////	processed all events 	////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "	////////////////////////////////////////////////////////////////////////////" << std::endl;
	streamlog_out(MESSAGE) << "	////////////////////////////////////////////////////////////////////////////" << std::endl;
	streamlog_out(MESSAGE) << " " << std::endl;

}