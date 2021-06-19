import React, { useState } from "react";
import { makeStyles } from '@material-ui/core/styles';
import './App.css';
import { BLESmartConfig, BLEUART, SSIDItem, IoTConfig } from 'ble-smartconfig';
import { Wizard, Steps, Step } from 'react-albus';
import { ConnectDevice } from "./ConnectDevice"
import { SelectSSID } from "./SelectSSID"
import { PassphraseForm } from "./PassphraseForm"
import { WiFiConnect } from "./WiFiConnect";
import { AWSConfigForm } from "./AWSConfigForm";
import { SendAWSConfig } from "./SendAWSConfig";

import Container from '@material-ui/core/Container';
import CssBaseline from '@material-ui/core/CssBaseline';
import Button from '@material-ui/core/Button';
import Typography from '@material-ui/core/Typography';


const useStyles = makeStyles((theme) => ({
  paper: {
    marginTop: theme.spacing(8),
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
  },
  avatar: {
    margin: theme.spacing(1),
    backgroundColor: theme.palette.secondary.main,
  },
  form: {
    width: '100%', // Fix IE 11 issue.
    marginTop: theme.spacing(1),
  },
  submit: {
    margin: theme.spacing(3, 0, 2),
  },
}));

function App() {
  const classes = useStyles();
  const [uart, setUart] = useState<BLEUART | undefined>(undefined);
  const [smartConfig, setSmartConfig] = useState<BLESmartConfig | undefined>(undefined);
  const [ssidItem, setSsidItem] = useState<SSIDItem | undefined>(undefined);
  const [passphrase, setPassphrase] = useState<string>("");

  const savedConfigStr = localStorage.getItem('iotConfig');
  let config = savedConfigStr ? JSON.parse(savedConfigStr) as IoTConfig : new IoTConfig();

  const [iotConfig, setIotConfig] = useState<IoTConfig>(config);

  return (
    <Container component="main" maxWidth="xs">
      <CssBaseline />
      <Wizard>
        <Steps>
          <Step
            id="merlin"
            render={({ next }) => (
              <div>
                <ConnectDevice onConnect={(uart: BLEUART) => {
                  setUart(uart);
                  setSmartConfig(new BLESmartConfig(uart));
                  next();
                }}/>
              </div>
            )}
          />
          <Step
            id="gandalf"
            render={({ next, previous }) => (
              <div>
                <SelectSSID smartConfig={smartConfig} onSelect={(ssidItem?: SSIDItem)=>{
                  setSsidItem(ssidItem);
                  next();
                }}/>
                <button onClick={previous}>Previous</button>
              </div>
            )}
          />
          <Step
            id="dumbledore"
            render={({ next, previous }) => (
              <div>
                {ssidItem ?
                <PassphraseForm ssidItem={ssidItem} onChange={(passphrase: string, validated: boolean) =>{
                  setPassphrase(passphrase);
                }}/>
                :<></>}
                <button onClick={previous}>Previous</button>
                <button onClick={next}>Next</button>
              </div>
            )}
          />
          <Step
            id="dumbledora"
            render={({ next, previous }) => (
              <div>
                {ssidItem ?
                <WiFiConnect smartConfig={smartConfig} ssidItem={ssidItem} passphrase={passphrase} onComplete={(connected: boolean) =>
                  next()
                }/>
                :<></>}
                <button onClick={previous}>Previous</button>
                <button onClick={next}>Next</button>
              </div>
            )}
          />
          <Step
            id="AWSConfigForm"
            render={({ next, previous }) => (
              <div>
                <AWSConfigForm config={iotConfig} onChange={(config: IoTConfig) => {
                  setIotConfig(config);
                  localStorage.setItem('iotConfig', JSON.stringify(config));
                }}/>
                <button onClick={previous}>Previous</button>
                <button onClick={next}>Next</button>
              </div>
            )}
          />
          <Step
              id="SendAWSConfig"
              render={({ next, previous }) => (
                <div>
                  <SendAWSConfig iotConfig={iotConfig} smartConfig={smartConfig} onComplete={(connected: boolean) =>
                    next()
                  }/>
                  <button onClick={previous}>Previous</button>
                </div>
              )}
            />
          <Step
              id="finished"
              render={({ previous }) => (
                <div>
                  Finished.
                  <button onClick={previous}>Previous</button>
                </div>
              )}
            />
        </Steps>
      </Wizard>
    </Container>
  );
}

export default App;
