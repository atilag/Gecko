[JSImplementation="@mozilla.org/dom/vzw-login-api;1",
 NavigatorProperty="vzwLogin"]
interface VZWLogin : EventTarget {
  Promise<DOMString> getToken();
  Promise<DOMString> deleteToken();
  // temporary test. it will be deleted
  Promise<DOMString> getAuthFromSim(DOMString authData);

  attribute EventHandler onmessage;
};
