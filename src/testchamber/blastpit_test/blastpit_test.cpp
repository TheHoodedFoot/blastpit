#include <cstdlib>

#include "blastpit_test.h"

/*! \class Blastpit blastpit.h "blastpit.h"
 *  \brief Main class for user blastpit operations.
 *  \define Blastpit is the high-level class containing
 */

BlastpitTest::BlastpitTest() {}

void
BlastpitTest::initTestCase()
{ /* Called before first test only */
}

void
BlastpitTest::cleanupTestCase()
{ /* Called after last test only */
}

void
BlastpitTest::init()
{ /* Called before every test */
}

void
BlastpitTest::cleanup()
{ /* Called after every test */
}

/*! \fn bool Blastpit::Listen( int port )
    \brief Activate listening server.
    \param [in] port Port number for listening server
    \return Return bool True if listen succeeds, false otherwise
    \define Begins a listening server on the specified TCP port. The server
   only accepts bpPackets with the correct checksum.
 */
void
BlastpitTest::listen()
{       /* Tests blastpit listening server
	 *
	 * What does the listening server do?
	 *
	 * 	Listens for TCP packets on a port
	 * 	Receives packets and strips invalid padding.
	 * 	Verifies checksum and acknowledges with good/bad reply
	 *    Queues packets for later processing
	 *    Emits receivedPacket() signal if good packet received
	 */
	/* QFAIL("Unimplemented"); */
}

QTEST_MAIN(BlastpitTest)

// bool Blastpit::Listen(port)
// bool Blastpit::Hangup(port)
// bool Blastpit::Connect(ip, port)
// bool Blastpit::Disconnect()
// bool Blastpit::UploadSVG(char*)
// bool Blastpit::DownloadSVG(char*)
// int Blastpit::GetReply(id) // gets the return value of the reply or false
// if not available bool Blastpit::TransferToLaser() // Converts the current
// SVG to Rofin XML and sends to the laser bool Blastpit::Burn(recipe) bool
// Blastpit::Preview(object? layer/path/whole drawing) null cancels current
// preview bool Blastpit::Stop() // Cancels burn in progress
//
// Store SVG internally as pugi but only expose strings or char*

/* uploads svg, parses geometries, recipes */
/* bp.connect("rofin", 9999) */
/* bp.burn("Recipe1") */
