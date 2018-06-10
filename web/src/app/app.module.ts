import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';

import { AppRoutingModule } from './app-routing.module';
import { AppComponent } from './app.component';
import { AddressBarComponent } from './address-bar/address-bar.component';
import { NodeListComponent } from './node-list/node-list.component';
import { PluginListComponent } from './plugin-list/plugin-list.component';
import { MonitorComponent } from './monitor/monitor.component';

@NgModule({
  declarations: [
    AppComponent,
    AddressBarComponent,
    NodeListComponent,
    PluginListComponent,
    MonitorComponent
  ],
  imports: [
    BrowserModule,
    FormsModule,
    AppRoutingModule,
    HttpClientModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
