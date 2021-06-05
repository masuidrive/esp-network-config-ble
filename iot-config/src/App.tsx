import React, { useState } from "react";
import { makeStyles } from '@material-ui/core/styles';
import './App.css';
import { BLESmartConfig, BLEUART, SSIDItem } from 'ble-smartconfig';
import { Wizard, Steps, Step } from 'react-albus';
import { ConnectDevice } from "./ConnectDevice"
import { SelectSSID } from "./SelectSSID"
import { PassphraseForm } from "./PassphraseForm"


import Container from '@material-ui/core/Container';
import Button from '@material-ui/core/Button';
import CssBaseline from '@material-ui/core/CssBaseline';
import { WiFiConnect } from "./WiFiConnect";


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
            render={({ previous }) => (
              <div>
                {ssidItem ?
                <WiFiConnect smartConfig={smartConfig} ssidItem={ssidItem} passphrase={passphrase} onComplete={(passohrase: string) =>{

                }}/>
                :<></>}
              </div>
            )}
          />
        </Steps>
      </Wizard>
    </Container>
  );
}

export default App;
