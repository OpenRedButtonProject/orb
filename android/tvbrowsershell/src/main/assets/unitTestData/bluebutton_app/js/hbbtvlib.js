/*
 * To use This library, You need to add the OIPF application manager objects to your HTML page,
 * 
 *	<object id="oipfAppMan" type="application/oipfApplicationManager"></object>
 */

function hbbtvlib_red_initialize(){
  //should be called show() function, if not the application will not be shown;
  var appManager = document.getElementById("oipfAppMan").getOwnerApplication(document);
  appManager.show();
	
  // IMPORTANT!!: only RED button should be accepted.
  //appManager.privateData.keyset.setValue(0x1);
};