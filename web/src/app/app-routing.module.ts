import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MonitorComponent } from './monitor/monitor.component';
import { PluginListComponent } from './plugin-list/plugin-list.component';

const routes: Routes = [
  { path: 'monitor/:id', component: MonitorComponent },
  { path: 'plugin-list', component: PluginListComponent }
];

@NgModule({
  imports: [RouterModule.forRoot(routes)],
  exports: [RouterModule]
})
export class AppRoutingModule { }
