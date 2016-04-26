
# when eqsys changes, this should be done somewhere
nVarStr=str(self.equationSystem.n_variables())
        self.numericalPanel.define('manifolds.evec1',"max="+nVarStr)
        self.numericalPanel.define('manifolds.evec2',"max="+nVarStr)

# when equationsystem structure changes
self.timeStamp=self.equationSystem.timestamp()
objects.move_cursor(None,self)

1f oder sowas gibt bei Eingabe keinen Fehler (Hexzahlen)
