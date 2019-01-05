#include <QString>
#include <QtTest>

#include <cassert>
#include <iostream>
#include <string>

#include "blastpit.h"
#include "layer.h"

class TestChamber : public QObject {
	Q_OBJECT

      public:
	TestChamber();

      private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void LayerTest();
	// void NetworkTest();
	/* void SimpleCombinedTest(); */
};

TestChamber::TestChamber() {}
void
TestChamber::initTestCase()
{
	asm("nop");
}
void
TestChamber::cleanupTestCase()
{
	asm("nop");
}
void
TestChamber::LayerTest()
{
	Blastpit bp;

	Layer layer1("Layer1");
	Layer layer2("Layer2", 10.0, 0x123456, 0xab, 0);

	Layer *layer3 = new Layer("unnamed");
	layer3->SetName("Layer3");
	layer3->SetHeight(20);
	layer3->SetColour(0xabcdef);
	layer3->SetLinetype(0xcd);
	layer3->SetIsLaserable(0);

	bp.AddLayer(&layer1);
	bp.AddLayer(&layer2);
	bp.AddLayer(layer3);

	Layer *layer3_alt = bp.GetLayerByName("Layer3");

	QVERIFY(layer1.GetName() == "Layer1");
	QVERIFY(layer2.GetName() == "Layer2");
	QVERIFY(layer2.GetHeight() == 10.0);
	QVERIFY(layer2.GetColour() == 0x123456);
	QVERIFY(layer3_alt->GetName() == "Layer3");
	QVERIFY(layer3_alt->GetHeight() == 20);
	QVERIFY(layer3_alt->GetColour() == 0xabcdef);
	QVERIFY(layer3_alt->GetLinetype() == 0xcd);
	QVERIFY(layer3_alt->GetIsLaserable() == 0);
	QVERIFY(bp.GetLayerByName("unnamed") == NULL);
	QVERIFY(bp.GetLayerCount() == 3);

	delete layer3;
}

/* void TestChamber::SimpleCombinedTest() */
/* { */
/*         Blastpit bp; */

/*         Layer layer1("Test Layer", 10.0, 0xFF7700, 0, 0); */
/*         Path path; */
/*         QVERIFY(path.Circle(60, 60, 10)); */
/*         QVERIFY(path.SetLayer(layer1)); */
/*         QVERIFY(path.SetHatchType("Argent")); */
/*         QVERIFY(path.SetMarkingType("Silver_With_Dark_Cleaning_Pass")); */
/*         QVERIFY(bp.Connect("laser.rfbevan.co.uk", 1234)); */
/*         int connect_status = bp.IsConnected(); */
/*         QVERIFY(connect_status); */
/*         if (connect_status) { */
/*                 QVERIFY(bp.Burn()); */
/*                 QVERIFY(bp.GetLiveHeight() == 10.0); */
/*                 QVERIFY(bp.OpenDoor()); */
/*         } */
/* } */

QTEST_APPLESS_MAIN(TestChamber)

#include "testchamber.moc"
