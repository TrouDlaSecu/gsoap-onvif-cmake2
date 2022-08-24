#include <iostream>
#include <string>
#include "stdio.h"
#include "stdlib.h"
#include "wsdd.nsmap"
#include "plugin/wsseapi.h"
#include "plugin/wsaapi.h"
#include  <openssl/rsa.h>
#include  "ErrorLog.h"
#include "soapStub.h" e
#include "plugin/wsaapi.h"

#include "include/soapDeviceBindingProxy.h"
#include "include/soapMediaBindingProxy.h"
#include "include/soapPTZBindingProxy.h"

#include "include/soapPullPointSubscriptionBindingProxy.h"
#include "include/soapRemoteDiscoveryBindingProxy.h" 

using namespace std;

#define MAX_DEVICE 20 

#define MAX_HOSTNAME_LEN 128
#define MAX_LOGMSG_LEN 256 


// Structure "device" utilisé après la découverte des caméra sur le réseau. Interrogation la sécurité de username et password. 
struct device {
	char brand;
	char hardware;
	char name;
	char profile;
	char username;
	char password;
	char MACAdress[20];
	char device_adress;	
};


// Fonction affichant l'erreur soap.
void PrintErr(struct soap* _psoap)
{
	fflush(stdout);
	cout << "ERROR : " << _psoap->error << " fault string : " << *soap_faultstring(_psoap) << " fault code : " << *soap_faultcode(_psoap) << " fault subcode : " << *soap_faultsubcode(_psoap) << "fault detail : " << *soap_faultdetail(_psoap) << endl;
}

int main(int argc, char* argv[])
{
	// **** DETECTION DES CAMERAS ****
	// déclaration des variables pour la recherche de caméras
	struct soap* soap; // création de la structure SOAP utilisé tout au long du programme, se référer au code XML pour comprendre qu'il s'agit du support de l'envoie des requêtes.
	struct wsdd__ProbeType req; //structure de données contenant le Probe Message encvoyé
	struct wsdd__ProbeType wsdd__Probe; // structure de données contenant le Probe Message servant de référence
	struct __wsdd__ProbeMatches resp; // structure de données contenant les Probe Matches : liste des Probes Matches reçu par le client 
	struct wsdd__ProbeMatchType *probeMatchType; // structure 
	struct wsdd__ScopesType sScope; // création de la structure permettant la portée de la requête
	struct SOAP_ENV__Header header; // création du Header pour l'envoie de la requête Probe Message

	int result = 0; // Variable contenant le resultat de l'envoie de Probe Message et de la reception  de Probe Matches 
	
	char _HwId[1024];


	soap = soap_new(); //initialisation de la strucuture soap

	// vérification que la structure soap est bien initialisée
	if (soap == NULL) {
		cout << "Error in creating new soap structure\n";
	}

	soap_set_namespaces(soap, namespaces);

	soap->recv_timeout = 5; // 5s timeout pour la reception de Probe Match

	soap_default_SOAP_ENV__Header(soap, &header); // ajout du header de base a la structure soap
	header.wsa__MessageID = _HwId; //Ajout de Message ID au header
	header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery"; // Destionation du message, schema repris pour l'envois
	header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"; // On specifie l'action : Probe Message

	soap->header = &header; // affectation du header crée au header de la structure soap

	soap_default_wsdd__ScopesType(soap, &sScope); // ajout de Scope de base à la structure soap
	sScope.__item = "";

	soap_default_wsdd__ProbeType(soap, &req); // ajout de la structure Probe de base à la structure soap
	req.Scopes = &sScope; // scope associée à la requete Probe Message

	req.Types = "tds:Device"; // definie le type d'objet que l'on va découvrir


	result = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702", NULL, &req);  
	// envoie Probe Message en udp à 239.255.255.250 adresse faite pour l'envoie local en multicast sur le port UDP 3702


	// vérification si le message est bien envoyé
	if (result != 0)
	{
		cout << "Error in sending probe message ! \n";
	}

	else 
	{

		cout << "Sended with sucess !! \n";

		cout << "Starting to receive ProbeMatches\n";

		// Réception de message tant que l'on a pas d'erreur de timeout
		while(SOAP_OK == result)
		{
			result = soap_recv___wsdd__ProbeMatches(soap, &resp); //fonction pour recevoir les Probe Matches
			if (SOAP_OK == result) // Cas de bonne réception
			{
				if (soap->error) // Cas de bonne réception mais avec une erreur
				{
					cout << "Error in receiving probe message ! \n";
				}

				else // Affichage des informations accessibles depuis le Probe Matche (contient l'adresse IP permettant l'envoie de messages 
				{
					cout << "\nSize ProbeMatches : " << resp.wsdd__ProbeMatches->__sizeProbeMatch; // Taille de la liste des Probe Match reçu
					cout << "\nTarget EP Adress : " << (std::string)resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address; // Adresse MAC découverte
					cout << "\nTarget Type : " << resp.wsdd__ProbeMatches->ProbeMatch->Types; // Type de caméra découverte
					cout << "\nTarget Service Adress : " << resp.wsdd__ProbeMatches->ProbeMatch->XAddrs; // Adresse pour se connecter en ONVIF à la caméra (contient son adresse IP)
					//cout << "\nTarget Metadata Version : " << resp.wsdd__ProbeMathes->ProbeMatch->MetadataVersion;
					cout << "\nTarget Scopes Adresses : " << resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item; // Contient marque, modele, profil ONVIF
					cout << "\n \n";


					// A ajouter : code pour créer la liste des structure type "device" au nombe de probe matches + gestion pour que ça soit joliment affiché 

					
				}
			resp = {}; // réinitialisation de la structure de Probe Matches pour détecter, si c'est possible, une nouvelle caméra
			}
			else if (soap->error) // cas d'erreur de timeout (ou mauvaise reception)
			{
				break;
			}


		}
	}

	soap_free(soap); //Libère la structure soap du contexte actuel (WS-Discovery)	



	// **** CONNEXION A UNE CAMERA ****
	
	// contient l'adresse ONVIF de connextion a la caméra
	char szHostName[MAX_HOSTNAME_LEN] = { 0 };
	// Adresse IP entrée par l'utilisateur
	char IP_Adrr[11];
	// Mot de passe de la caméra à laquelle on se connecte
	char mdp[MAX_HOSTNAME_LEN];
	//Adresse RTSP du stream pour l'utiliser pour lancer le stream
	string StreamUri = "";

	// initialisation des différentes proxy classes 
	DeviceBindingProxy proxyDevice;
	RemoteDiscoveryBindingProxy proxyDiscovery;
	MediaBindingProxy proxyMedia;
	PTZBindingProxy proxyPTZ;
	PullPointSubscriptionBindingProxy proxyEvent;

	//enregistrement des plugines
	soap_register_plugin(proxyDevice.soap, soap_wsse);
	soap_register_plugin(proxyDiscovery.soap, soap_wsse);
	soap_register_plugin(proxyMedia.soap, soap_wsse);
	soap_register_plugin(proxyPTZ.soap, soap_wsse);
	soap_register_plugin(proxyEvent.soap, soap_wsse);
	soap_register_plugin(proxyEvent.soap, soap_wsa);

	soap = soap_new();

	
	cout << "Entrez IP adress de IPC à se connecter\n"; 
	cin >> IP_Adrr;

	// reconstitution de l'adresse de connexion ONVIF (en créant les objets des caméra on pourra effacer cette partie
	strcat(szHostName, "http://");
	strcat(szHostName, IP_Adrr);
	strcat(szHostName, "/onvif/device_service");

	// On associe l'adresse de connexion pour les messages destiné à la classe Device et tout ce qui y est liée (il faut faire la même chose pour les autres classes)
	proxyDevice.soap_endpoint = szHostName;

	cout << "Entrez le mdp de la caméra" << endl;
	cin >> mdp;



	// INFOS DE LA CAMERA
	/* Je ne définierai pas toute la suite du code pour récuperer les différentes informations de la caméra car le code se ressemble mais seulement les classes 
	appelées et les fonctions utilisées diffèrent. Ce qu'il faut retenir c'est le mode requête/réponse, l'ajout d'authentification et de durée de vie de message avant 
	l'utilisation de certaines fonction.*/


	cout << "\n\n **** INFOS IPC ****\n\n";

	// Création des classes une pour l'envoie et une pour la reception (ici c'est pour la partie Information de la caméra
	// Appels des fonction permettant d'initialiser les objets si ils ont déja été utilisés
	_tds__GetDeviceInformation* GetDeviceInformation = soap_new__tds__GetDeviceInformation(soap, -1); 
	_tds__GetDeviceInformationResponse* GetDeviceInformationResponse = soap_new__tds__GetDeviceInformationResponse(soap, -1);

	// cryptage du message avec authentification on met en option l'adresse du destinataire et si besoin identifiant et mot de passe
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	//ajout d'un timestamp pour déterminer la durée de validitée du message
	if (SOAP_OK != soap_wsse_add_Timestamp(proxyDevice.soap, "Time", 10))
	{
		return -1;
	}

	// en cas de bonne reception du message et/ou que le format du message est selon la norme alors on affiche son contenu
	if (SOAP_OK == proxyDevice.GetDeviceInformation(GetDeviceInformation, GetDeviceInformationResponse))
	{
		cout << "Manufacturer : " << GetDeviceInformationResponse->Manufacturer.c_str() << endl;
		cout << "Model : " << GetDeviceInformationResponse->Model << endl;
		cout << "Firmware Version : " << GetDeviceInformationResponse->FirmwareVersion.c_str() << endl;
		cout << "Serial Number : " << GetDeviceInformationResponse->SerialNumber << endl;
		cout << "Hardware ID : " << GetDeviceInformationResponse->HardwareId.c_str() << endl;

	}

	// Permet d'effacer le contexte et le contenu du message pour et son support afin utiliser toujours la seule structure soap définie
	soap_destroy(soap);
	soap_end(soap);

	_tds__GetCapabilities* GetCapabilities = soap_new__tds__GetCapabilities(soap, -1);
	GetCapabilities->Category.push_back(tt__CapabilityCategory__All);

	_tds__GetCapabilitiesResponse* GetCapabilitiesResponse = soap_new__tds__GetCapabilitiesResponse(soap, -1);

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyDevice.soap, "Time", 10))
	{
		return -1;
	}

	if (SOAP_OK == proxyDevice.GetCapabilities(GetCapabilities, GetCapabilitiesResponse))
	{
		if (GetCapabilitiesResponse->Capabilities->Analytics != NULL)
		{
			cout << "\n**** ANALYTICS INFOS ****\n" << endl;
			cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->Analytics->XAddr.c_str() << endl;
			cout << "Rule Support : " << (GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport ? "yes" : "no") << endl;
			cout << "Analytics Module Support : " << (GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport ? "yes" : "no") << endl;
		}
	}


	if (GetCapabilitiesResponse->Capabilities->Device != NULL)
	{
		cout << "\n**** DEVICE INFOS ****\n" << endl;
		cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->Device->XAddr.c_str() << endl;
		cout << "IP Filter : " << (GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration ? "yes" : "no") << endl;
		cout << "IPv6 : " << (GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 ? "yes" : "no") << endl;
		cout << "DynDNS : " << (GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS ? "yes" : "no") << endl;
		cout << "Discovery Resolve : " << (GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve ? "yes" : "no") << endl;
		cout << "Discovery Bye : " << (GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye ? "yes" : "no") << endl;
		cout << "Remote Discovery : " << (GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery ? "yes" : "no") << endl;

		int supportedVersion = GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions.size();

		if (supportedVersion > 0)
		{

			for (int i = 0; i < supportedVersion; i++)
			{
				cout << "Supported Version : " << GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Major << "." << GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[i]->Minor << endl;
			}


		}


		cout << "System Backup : " << (GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup ? "yes" : "no") << endl;
		cout << "Firmware Upgrade : " << (GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade ? "yes" : "no") << endl;
		cout << "System Logging : " << (GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging ? "yes" : "no") << endl;
		cout << "Input Connectors : " << GetCapabilitiesResponse->Capabilities->Device->IO->InputConnectors << endl;
		cout << "Relay Outputs : " << GetCapabilitiesResponse->Capabilities->Device->IO->RelayOutputs << endl;
		cout << "TLS 1.1 : " << (GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 ? "yes" : "no") << endl;
		cout << "TLS 1.2 : " << (GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 ? "yes" : "no") << endl;
		cout << "Onboard Key Generation : " << (GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration ? "yes" : "no") << endl;
		cout << "Access Policy Config : " << (GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig ? "yes" : "no") << endl;
		cout << "X.509 Token : " << (GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token ? "yes" : "no") << endl;
		cout << "SAML Token : " << (GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken ? "yes" : "no") << endl;
		cout << "Kerberos Token : " << (GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken ? "yes" : "no") << endl;
		cout << "REL Token : " << (GetCapabilitiesResponse->Capabilities->Device->Security->RELToken ? "yes" : "no") << endl;
	}

	if (GetCapabilitiesResponse->Capabilities->Events != NULL)
	{
		cout << "\n**** EVENTS INFOS ****\n" << endl;
		cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str() << endl;
		cout << "WS-Subscription Policy Support : " << (GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport ? "yes" : "no") << endl;
		cout << "WS-Pull Point Support : " << (GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport ? "yes" : "no") << endl;
		cout << "WS-Pausable Subscription Manager Interface Support : " << (GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport ? "yes" : "no") << endl;
		proxyEvent.soap_endpoint = GetCapabilitiesResponse->Capabilities->Events->XAddr.c_str();

	}

	if (GetCapabilitiesResponse->Capabilities->Imaging != NULL)
	{
		cout << "\n**** IMAGING INFOS ****\n" << endl;
		cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->Imaging->XAddr.c_str() << endl;
	}

	if (GetCapabilitiesResponse->Capabilities->Media != NULL)
	{
		cout << "\n **** MEDIA INFOS ****\n" << endl;
		cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str() << endl;
		cout << "RTP Multicast : " << (GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast ? "yes" : "no") << endl;
		cout << "RTP_TCP : " << (GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP ? "yes" : "no") << endl;
		cout << "RTP_RTSP_TCP : " << (GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP ? "yes" : "no") << endl;

		proxyMedia.soap_endpoint = GetCapabilitiesResponse->Capabilities->Media->XAddr.c_str();
	}

	if (GetCapabilitiesResponse->Capabilities->PTZ != NULL)
	{
		cout << "\n**** PTZ INFOS ****\n" << endl;
		cout << "XAddr : " << GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str() << endl;

		proxyPTZ.soap_endpoint = GetCapabilitiesResponse->Capabilities->PTZ->XAddr.c_str();
	}

	soap_destroy(soap);
	soap_end(soap);

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyDevice.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	_tds__GetNetworkInterfaces* GetNetworkInterfaces = soap_new__tds__GetNetworkInterfaces(soap, -1);
	_tds__GetNetworkInterfacesResponse* GetNetworkInterfacesResponse = soap_new__tds__GetNetworkInterfacesResponse(soap, -1);

	if (proxyDevice.GetNetworkInterfaces(GetNetworkInterfaces, GetNetworkInterfacesResponse))
	{
		cout << "\n**** NETWORK INFOS ****\n" << endl;
		cout << "Token : " << GetNetworkInterfacesResponse->NetworkInterfaces[0]->token.c_str() << endl;
		cout << "HwAdress : " << GetNetworkInterfacesResponse->NetworkInterfaces[0]->Info->HwAddress.c_str() << endl;
	}

	soap_destroy(soap);
	soap_end(soap);

	
	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10))
	{
		return -1;
	}

	_trt__GetProfiles* GetProfiles = soap_new__trt__GetProfiles(soap, -1);
	_trt__GetProfilesResponse* GetProfilesResponse = soap_new__trt__GetProfilesResponse(soap, -1);

	if (SOAP_OK == proxyMedia.GetProfiles(GetProfiles, GetProfilesResponse))
	{

		_trt__GetStreamUri* GetStreamUri = soap_new__trt__GetStreamUri(soap, -1);
		GetStreamUri->StreamSetup = soap_new_tt__StreamSetup(soap, -1);
		GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
		GetStreamUri->StreamSetup->Transport = soap_new_tt__Transport(soap, -1);
		GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;

		_trt__GetStreamUriResponse* GetStreamUriResponse = soap_new__trt__GetStreamUriResponse(soap, -1);





		// Code permettant d'avoir les adresses des streams RTSP des différents profiles de la caméra
		cout << "\n**** MEDIA PROFILES ****\n" << endl;

		for (int i = 0; i < GetProfilesResponse->Profiles.size(); i++)
		{
			cout << "Profile : " << i;
			cout << ":" << GetProfilesResponse->Profiles[i]->Name.c_str();
			cout << " Token : " << GetProfilesResponse->Profiles[i]->token.c_str() << endl;

			GetStreamUri->ProfileToken = GetProfilesResponse->Profiles[i]->token;



			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", mdp))
			{
				return -1;
			}

			if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10))
			{
				return -1;
			}

			if (SOAP_OK == proxyMedia.GetStreamUri(GetStreamUri, GetStreamUriResponse))
			{
				cout << " RTSP URI : " << GetStreamUriResponse->MediaUri->Uri.c_str() << endl;
				StreamUri = GetStreamUriResponse->MediaUri->Uri;
			}


			else
			{
				PrintErr(proxyMedia.soap);
			}
		}
	}

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyMedia.soap, "Time", 10))
	{
		return -1;
	}


	if (SOAP_OK == proxyMedia.GetProfiles(GetProfiles, GetProfilesResponse))
	{
		_trt__GetSnapshotUri* GetSnapshotUri = soap_new__trt__GetSnapshotUri(soap, -1);
		_trt__GetSnapshotUriResponse* GetSnapshotUriResponse = soap_new__trt__GetSnapshotUriResponse(soap, -1);

		cout << "\n**** SNAPSHOT ****\n" << endl;

		for (int i = 0; i < GetProfilesResponse->Profiles.size(); i++)
		{
			GetSnapshotUri->ProfileToken = GetProfilesResponse->Profiles[i]->token;

			if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyMedia.soap, NULL, "admin", mdp))
			{
				return -1;
			}

			if (SOAP_OK == proxyMedia.GetSnapshotUri(GetSnapshotUri, GetSnapshotUriResponse))
			{
				cout << "Profile Name : " << GetProfilesResponse->Profiles[i]->Name.c_str() << endl;
				cout << "Snapshot URI :" << GetSnapshotUriResponse->MediaUri->Uri.c_str() << endl;

				//Création du script permettant la récupération des captures d'écrans de la caméra
				char command[280] = { "./curlGetSnapshot.sh " };
				char name[18];
				char espace[10] = " ";


				cout << "Choississez le nom du fichier" << endl;
				cin >> name;

				strcat(command, GetSnapshotUriResponse->MediaUri->Uri.c_str());
				strcat(command, espace);
				strcat(command, name);
				cout << command << endl;

				cout << "Using script to download Snapshot" << endl;
				system(command);
				cout << "End of the script" << endl;



			}



		}
	}
	
	soap_destroy(soap);
	soap_end(soap);

	if (SOAP_OK != soap_wsse_add_UsernameTokenDigest(proxyEvent.soap, NULL, "admin", mdp))
	{
		return -1;
	}

	if (SOAP_OK != soap_wsse_add_Timestamp(proxyEvent.soap, "Time", 10))
	{
		return -1;
	}

	_tev__GetEventProperties* tev__GetEventProperties = soap_new__tev__GetEventProperties(soap, -1);
	_tev__GetEventPropertiesResponse* tev__GetEventPropertiesResponse = soap_new__tev__GetEventPropertiesResponse(soap, -1);

	if (SOAP_OK != soap_wsa_request(proxyEvent.soap, NULL, NULL, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesRequest"))
	{
		return -1;
	}

	if (proxyEvent.GetEventProperties(tev__GetEventProperties, tev__GetEventPropertiesResponse) == SOAP_OK)
	{
		cout << "\n**** EVENTS PROPERTIES ****\n" << endl;

		for (int i = 0; i < tev__GetEventPropertiesResponse->TopicNamespaceLocation.size(); i++)
		{
			cout << "Topic Namespace Location " << i << " : " << tev__GetEventPropertiesResponse->TopicNamespaceLocation[i].c_str() << endl;
		}

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentFilterDialect.size(); i++)
		{
			cout << "Message Content Filter Dialect " << i << " : " << tev__GetEventPropertiesResponse->MessageContentFilterDialect[i].c_str() << endl;
		}

		for (int i = 0; i < tev__GetEventPropertiesResponse->MessageContentSchemaLocation.size(); i++)
		{
			cout << "Message Content Schema Location " << i << " : " << tev__GetEventPropertiesResponse->MessageContentSchemaLocation.size() << endl;
		}

	}

	soap_destroy(soap);
	soap_end(soap);

	cout << "\n**** OPENING LOCAL STREAM ****\n" << endl;
	//code pour lancer stream local 
	
	char command1[280] = { "./PlayStream.sh " };
	string identification = "admin:" + (string)mdp + "@";
	string rtspAdress = StreamUri;
	rtspAdress = rtspAdress.insert(7, identification);
	int length = rtspAdress.length();
	char option[length + 1] = "";
	strcpy(option, rtspAdress.c_str());
	strcat(command1, option);
	system(command1);



	 
	 

	// CODE FOR OPEN CV -- Gestion d'évènements (détection image, son, ... librairie efficase pour implémenter une IA) 



		return 0;
}
