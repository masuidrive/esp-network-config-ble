import React, { useState } from "react";
import { makeStyles } from '@material-ui/core/styles';
import './App.css';
import { BLESmartConfig, BLEUART } from 'ble-smartconfig';
import { Wizard, Steps, Step } from 'react-albus';
import { ConnectDevice } from "./ConnectDevice"
import { SelectSSID } from "./SelectSSID"


import Container from '@material-ui/core/Container';
import Avatar from '@material-ui/core/Avatar';
import Button from '@material-ui/core/Button';
import CssBaseline from '@material-ui/core/CssBaseline';
import TextField from '@material-ui/core/TextField';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import Checkbox from '@material-ui/core/Checkbox';
import Link from '@material-ui/core/Link';
import Grid from '@material-ui/core/Grid';
import Box from '@material-ui/core/Box';
import LockOutlinedIcon from '@material-ui/icons/LockOutlined';
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
                <SelectSSID smartConfig={smartConfig} onSelect={()=>{
                  next();
                }}/>
                <button onClick={previous}>Previous</button>
              </div>
            )}
          />
          <Step
            id="dumbledore"
            render={({ previous }) => (
              <div>
                <h1>Dumbledore</h1>
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
