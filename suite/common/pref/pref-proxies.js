function showManualProxyConfig()
{
  var manualRow = document.getElementById("manual-proxy");
  bundle = srGetStrBundle("chrome://pref/locale/prefutilities.properties");
  var viewHideButton = document.getElementById("viewhideManual");
  if( manualRow.style.display == "none" ) {
    viewHideButton.value = bundle.GetStringFromName("hiderow");
    manualRow.style.display = "inherit";
  }
  else {
    viewHideButton.value = bundle.GetStringFromName("viewrow");
    manualRow.style.display = "none"
  }
}

function Startup()
{
  DoEnabling();
}

function DoEnabling()
{
  var ftp = document.getElementById("networkProxyFTP");
  var ftpPort = document.getElementById("networkProxyFTP_Port");
  var http = document.getElementById("networkProxyHTTP");
  var httpPort = document.getElementById("networkProxyHTTP_Port");
  var ssl = document.getElementById("networkProxySSL");
  var sslPort = document.getElementById("networkProxySSL_Port");
  var noProxy = document.getElementById("networkProxyNone");
  var autoURL = document.getElementById("networkProxyAutoconfigURL");
  var autoReload = document.getElementById("autoReload");

  // convenience arrays
  var manual = [ftp, ftpPort, http, httpPort, ssl, sslPort, noProxy];
  var auto = [autoURL, autoReload];
  
  // radio buttons
  var radiogroup = document.getElementById("networkProxyType");

  switch ( radiogroup.data ) {
    case "0":  
      for( var i = 0; i < manual.length; i++ ) 
        manual[i].setAttribute( "disabled", "true" );
      for( var i = 0; i < auto.length; i++ ) 
        auto[i].setAttribute( "disabled", "true" );
      break;
    case "1":
      for( var i = 0; i < auto.length; i++ ) 
        auto[i].setAttribute( "disabled", "true" );
      for( var i = 0; i < manual.length; i++ ) 
        manual[i].removeAttribute( "disabled" );
      break;
    case "2":
    default:
      for( var i = 0; i < manual.length; i++ ) 
        manual[i].setAttribute( "disabled", "true" );
      for( var i = 0; i < auto.length; i++ ) 
        auto[i].removeAttribute( "disabled" );
      break;
  }
}